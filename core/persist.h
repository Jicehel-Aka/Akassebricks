/**
 * @file persist.h
 * @brief Interface de sauvegarde persistante via NVS (Non‑Volatile Storage).
 *
 * Ce module fournit une API simple pour stocker et charger des données
 * persistantes dans la mémoire interne de l’ESP32‑S3, indépendamment de la
 * carte SD. Il est utilisé pour sauvegarder une liste de HighScore sous forme
 * de blob binaire.
 *
 * Avantages :
 *   - Stockage interne fiable (pas besoin de carte SD).
 *   - Lecture/écriture atomique.
 *   - Idéal pour les petites données (scores, options, préférences).
 *
 * Limites :
 *   - Taille limitée (quelques Ko).
 *   - Pas adapté aux gros fichiers (préférer FATFS pour cela).
 *
 * Ce module est totalement indépendant du système de fichiers SD.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

/**
 * @struct HighScore
 * @brief Structure représentant un score sauvegardé dans NVS.
 *
 * Champs :
 *   - name  : nom du joueur (std::string)
 *   - score : score associé (int)
 *
 * Cette structure est sérialisée telle quelle dans NVS via memcpy().
 */
struct HighScore {
    char name[9];       // compatible avec HighscoreEntry
    int32_t score;
};

/**
 * @brief Charge depuis NVS la liste des scores persistants.
 *
 * @param[out] scores
 *     Le vecteur sera rempli avec les scores chargés.
 *     Si aucune donnée n’est présente, le vecteur reste vide.
 *
 * Fonctionnement :
 *   - ouvre NVS en lecture,
 *   - lit la taille du blob,
 *   - lit le blob complet,
 *   - reconstruit les HighScore.
 */
void persist_load(std::vector<HighScore>& scores);

/**
 * @brief Sauvegarde dans NVS la liste des scores persistants.
 *
 * @param[in] scores
 *     Le vecteur de scores à sauvegarder.
 *
 * Fonctionnement :
 *   - ouvre NVS en écriture,
 *   - encode le vecteur en blob binaire,
 *   - écrit le blob,
 *   - commit pour valider.
 */
void persist_save(const std::vector<HighScore>& scores);
