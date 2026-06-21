/**
 * @file sdcard.c
 * @brief Gestion de la carte SD pour la console AKA (ESP32‑S3) via SDMMC (bus natif 4 bits) + FATFS.
 *
 * IMPORTANT — ESP‑IDF 5.x :
 *   - Le support LFN (Long File Names) est intégré directement dans FILINFO.fname[].
 *   - Il n’existe plus de champs lfname / lfsize.
 *   - Aucun buffer externe n’est nécessaire pour les noms longs.
 *
 * NOTE MATÉRIELLE (corrigé) :
 *   - La carte SD de la console AKA est câblée en SDMMC natif (CMD/CLK/D0-D3),
 *     PAS en SPI. L'ancienne implémentation utilisait SDSPI sur les GPIO 10-13,
 *     qui sont en réalité le bus de données parallèle du LCD (LCD_PIN_DB3..DB6) :
 *     les deux périphériques se disputaient les mêmes broches, ce qui empêchait
 *     tout montage de la carte SD (sd_init() retournait systématiquement false).
 */

#include "sdcard.h"
#include "esp_log.h"

#include "ff.h"              // doit être avant esp_vfs_fat.h
#include "esp_vfs_fat.h"

#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"

static const char *TAG = "SDCARD";

// -----------------------------------------------------------------------------
//  Configuration matérielle SDMMC (AKA console, bus natif 4 bits)
// -----------------------------------------------------------------------------
#define SDCARD_PIN_CMD 38
#define SDCARD_PIN_CLK 2
#define SDCARD_PIN_D0  1
#define SDCARD_PIN_D1  15
#define SDCARD_PIN_D2  7
#define SDCARD_PIN_D3  16

static sdmmc_card_t *card = NULL;

// -----------------------------------------------------------------------------
//  sd_init()
// -----------------------------------------------------------------------------
bool sd_init()
{
    ESP_LOGI(TAG, "Initialisation SD (SDMMC 4 bits + FATFS)");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
#ifdef CONFIG_SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = SDCARD_PIN_CLK;
    slot_config.cmd = SDCARD_PIN_CMD;
    slot_config.d0  = SDCARD_PIN_D0;
    slot_config.d1  = SDCARD_PIN_D1;
    slot_config.d2  = SDCARD_PIN_D2;
    slot_config.d3  = SDCARD_PIN_D3;
#endif // CONFIG_SOC_SDMMC_USE_GPIO_MATRIX

    // Pull-ups internes activées ; des résistances de pull-up externes (10k)
    // sur le bus restent recommandées en production.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Echec du montage du systeme de fichiers");
        } else {
            ESP_LOGE(TAG, "Echec d'initialisation de la carte (%s)", esp_err_to_name(ret));
        }
        return false;
    }

    ESP_LOGI(TAG, "SD OK, FATFS monté");
    return true;
}

// -----------------------------------------------------------------------------
//  sd_mkdir()
// -----------------------------------------------------------------------------
bool sd_mkdir(const char *path)
{
    FRESULT fr = f_mkdir(path);
    return (fr == FR_OK || fr == FR_EXIST);
}

// -----------------------------------------------------------------------------
//  sd_exists()
// -----------------------------------------------------------------------------
bool sd_exists(const char *path)
{
    FILINFO info;
    FRESULT fr = f_stat(path, &info);
    return (fr == FR_OK);
}

// -----------------------------------------------------------------------------
//  sd_list_dir()
// -----------------------------------------------------------------------------
bool sd_list_dir(const char *path)
{
    FF_DIR dir;
    FILINFO info;

    if (f_opendir(&dir, path) != FR_OK)
        return false;

    while (true) {
        FRESULT fr = f_readdir(&dir, &info);
        if (fr != FR_OK || info.fname[0] == 0)
            break;

        // ESP‑IDF 5.x : fname contient déjà le nom long
        ESP_LOGI("SDCARD", "Entry: %s", info.fname);
    }

    f_closedir(&dir);
    return true;
}

// -----------------------------------------------------------------------------
//  Gestion des fichiers WAV (restauré depuis ancienne version)
// -----------------------------------------------------------------------------
static FILE *f_wav = NULL;
static uint32_t filesize = 0;
static uint32_t filepos = 0;

esp_err_t open_file(const char *path)
{
    printf("Loading file %s\n", path);
    f_wav = fopen(path, "r");
    if (f_wav == NULL)
    {
        ESP_LOGE("SDCARD", "Failed to open for reading");
        return ESP_FAIL;
    }

    fseek(f_wav, 0, SEEK_END);
    filesize = ftell(f_wav);
    printf("File size %ld \n", filesize);
    fseek(f_wav, 0, SEEK_SET);
    filepos = 0;

    return ESP_OK;
}

void read_audio_file(uint8_t *ptr, uint32_t u32_length)
{
    if (!f_wav)
        return;

    uint32_t u32_read = filesize - filepos;
    if (u32_read > u32_length)
        u32_read = u32_length;

    fread(ptr, u32_read, 1, f_wav);
    filepos += u32_read;

    if (filepos >= filesize)
    {
        fseek(f_wav, 0, SEEK_SET);
        filepos = 0;
    }
}

