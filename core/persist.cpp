/**
 * @file persist.cpp
 * @brief Gestion de la sauvegarde persistante via NVS (Non‑Volatile Storage).
 *
 * Ce module permet de stocker des données simples et durables dans la mémoire
 * interne de l’ESP32‑S3, indépendamment de la carte SD.
 *
 * Il est utilisé ici pour stocker une liste de HighScore sous forme de blob.
 *
 * Avantages de NVS :
 *   - Pas besoin de carte SD.
 *   - Sauvegarde interne, fiable et rapide.
 *   - Lecture/écriture atomique.
 *
 * Limites :
 *   - Taille limitée.
 *   - Pas adapté aux gros fichiers (préférer FATFS pour ça).
 *
 * Ce module est totalement indépendant du système de fichiers SD.
 */

#include "persist.h"
#include "nvs_flash.h"
#include "nvs.h"

#include <cstring>
#include <cstdio>

// Namespace et clé utilisés dans NVS
static const char* NVS_NAMESPACE = "storage";
static const char* NVS_KEY       = "highscores";

// -----------------------------------------------------------------------------
//  persist_load()
// -----------------------------------------------------------------------------
//  Charge depuis NVS un tableau de HighScore.
//
//  Fonctionnement :
//    - ouvre NVS en lecture,
//    - récupère la taille du blob,
//    - lit le blob dans un buffer,
//    - reconstruit le vecteur de HighScore.
//
//  Si aucune donnée n’est présente → scores reste vide.
// -----------------------------------------------------------------------------
void persist_load(std::vector<HighScore>& scores) {
    scores.clear();

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        printf("NVS open failed: %s\n", esp_err_to_name(err));
        return;
    }

    // Première lecture : obtenir la taille du blob
    size_t required_size = 0;
    err = nvs_get_blob(handle, NVS_KEY, NULL, &required_size);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Rien de stocké → tableau vide
        nvs_close(handle);
        return;
    }

    // Lecture réelle du blob
    std::vector<uint8_t> buffer(required_size);
    err = nvs_get_blob(handle, NVS_KEY, buffer.data(), &required_size);

    if (err == ESP_OK) {
        size_t count = required_size / sizeof(HighScore);
        scores.resize(count);
        memcpy(scores.data(), buffer.data(), required_size);
    }

    nvs_close(handle);
}

// -----------------------------------------------------------------------------
//  persist_save()
// -----------------------------------------------------------------------------
//  Sauvegarde un tableau de HighScore dans NVS.
//
//  Fonctionnement :
//    - ouvre NVS en écriture,
//    - encode le vecteur en blob,
//    - écrit le blob,
//    - commit pour valider,
//    - ferme NVS.
//
//  Remarque :
//    - NVS écrase automatiquement l’ancienne valeur.
// -----------------------------------------------------------------------------
void persist_save(const std::vector<HighScore>& scores) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        printf("NVS open failed: %s\n", esp_err_to_name(err));
        return;
    }

    size_t size = scores.size() * sizeof(HighScore);

    // Écriture du blob
    err = nvs_set_blob(handle, NVS_KEY, scores.data(), size);
    if (err != ESP_OK) {
        printf("NVS set_blob failed: %s\n", esp_err_to_name(err));
    }

    // Validation
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        printf("NVS commit failed: %s\n", esp_err_to_name(err));
    }

    nvs_close(handle);
}
