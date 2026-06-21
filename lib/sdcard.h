/**
 * @file sdcard.h
 * @brief Gestion de la carte SD pour la console AKA (ESP32‑S3) via SDSPI + FATFS.
 *
 * Ce module fournit une interface simple et unifiée pour accéder à la carte SD :
 *
 *   - Initialisation du bus SPI et montage du système de fichiers FATFS.
 *   - Support des noms longs (LFN) si activé dans sdkconfig.
 *   - Création de répertoires.
 *   - Vérification de l’existence de fichiers ou dossiers.
 *   - Listing du contenu d’un répertoire.
 *   - Lecture de fichiers WAV pour l’audio.
 *
 * Notes importantes :
 *   - Ce module utilise **SDSPI**, compatible avec le câblage de la console AKA.
 *   - Le point de montage utilisé est toujours : "/sdcard".
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"   // pour esp_err_t

#ifdef __cplusplus
extern "C" {
#endif

// --- Fonctions SD classiques ---
bool sd_init();
bool sd_mkdir(const char *path);
bool sd_exists(const char *path);
bool sd_list_dir(const char *path);

// --- Fonctions audio restaurées ---
/**
 * @brief Ouvre un fichier WAV sur la carte SD.
 *
 * @param path Chemin complet du fichier WAV.
 * @return ESP_OK si succès, ESP_FAIL sinon.
 */
esp_err_t open_file(const char *path);

/**
 * @brief Lit un bloc de données audio depuis le fichier WAV ouvert.
 *
 * @param ptr Pointeur vers le buffer de destination.
 * @param u32_length Nombre d’octets à lire.
 */
void read_audio_file(uint8_t *ptr, uint32_t u32_length);

#ifdef __cplusplus
}
#endif
