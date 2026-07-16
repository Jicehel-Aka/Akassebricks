/*
  core/i18n.h — Localisation AKasseBricks (5 langues), sur le modele de mAKArena.
  La police du composant etant ASCII, les textes sont volontairement SANS accent.
  La persistance (langue+volume) est sur SD, voir core/settings.
*/
#pragma once
#include <cstdint>

namespace i18n {

enum Lang : uint8_t { FR = 0, EN, DE, ES, IT, LANG_COUNT };

enum Str : uint16_t {
    STR_PRESS_A_START = 0,
    STR_MENU,
    STR_PLAY,
    STR_RESUME,
    STR_OPTIONS,
    STR_VOLUME,
    STR_LANGUAGE,
    STR_SCORES,
    STR_HIGHSCORES,
    STR_NO_SCORE,
    STR_CONTROLS,
    STR_RECALIBRATE,
    STR_SCREENSHOT,
    STR_RETURN_LOADER,
    STR_BACK,
    STR_NAME,
    STR_NAME_HELP,
    STR_GAMEOVER,
    STR_PRESS_A_RETRY,
    STR_PAUSE,
    STR_PRESS_A_RESUME,
    STR_EDITOR,
    STR_SHOT_SAVED,
    STR_LANG_NAME,          // nom de la langue courante (Francais, English...)
    STR_COUNT
};

// Renvoie la chaine traduite dans la langue courante.
const char* T(Str s);

void  set_language(Lang l);
Lang  get_language();
void  next_language();      // cycle FR -> EN -> DE -> ES -> IT -> FR


} // namespace i18n
