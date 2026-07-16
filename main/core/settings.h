/*
  core/settings.h — Reglages persistants sur la carte SD (PAS en NVS).
  Fichier : /sdcard/AKAsseBricks/SETTINGS.DAT  (meme dossier que les scores).
  Contient pour l'instant : langue + volume. Extensible ensuite.
*/
#pragma once

void settings_load();   // lit le fichier et applique langue + volume (defauts sinon)
void settings_save();   // ecrit langue + volume courants
