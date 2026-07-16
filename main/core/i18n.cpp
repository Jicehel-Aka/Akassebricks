/*
  core/i18n.cpp — Tables de traduction. La persistance (langue) est geree par core/settings (SD).
  Textes SANS accent (police ASCII du composant).
*/
#include "i18n.h"

namespace i18n {

// Table [langue][chaine]. L'ordre des colonnes suit l'enum Str.
static const char* K[LANG_COUNT][STR_COUNT] = {
  // ---- FR ----
  {
    "Appuyez sur A pour jouer", "Menu", "Jouer", "Reprendre", "Options",
    "Volume", "Langue", "Scores", "Meilleurs scores", "Aucun score",
    "Commandes", "Recalibrer le stick", "Capture d'ecran", "Retour au loader", "B : retour",
    "Pseudo", "A: lettre  C: effacer  B: ok", "Partie terminee", "A : rejouer",
    "Pause", "A : reprendre", "Editeur", "Capture enregistree", "Francais"
  },
  // ---- EN ----
  {
    "Press A to play", "Menu", "Play", "Resume", "Options",
    "Volume", "Language", "Scores", "High scores", "No score yet",
    "Controls", "Recalibrate stick", "Screenshot", "Back to loader", "B: back",
    "Name", "A: letter  C: erase  B: ok", "Game over", "A: play again",
    "Pause", "A: resume", "Editor", "Screenshot saved", "English"
  },
  // ---- DE ----
  {
    "A druecken zum Spielen", "Menue", "Spielen", "Weiter", "Optionen",
    "Lautstaerke", "Sprache", "Punkte", "Bestenliste", "Kein Ergebnis",
    "Steuerung", "Stick kalibrieren", "Screenshot", "Zum Loader", "B: zurueck",
    "Name", "A: Buchst.  C: loesch  B: ok", "Spiel vorbei", "A: nochmal",
    "Pause", "A: weiter", "Editor", "Screenshot gespeichert", "Deutsch"
  },
  // ---- ES ----
  {
    "Pulsa A para jugar", "Menu", "Jugar", "Seguir", "Opciones",
    "Volumen", "Idioma", "Puntos", "Mejores puntos", "Sin puntos",
    "Controles", "Calibrar mando", "Captura", "Volver al loader", "B: atras",
    "Nombre", "A: letra  C: borrar  B: ok", "Fin de partida", "A: reintentar",
    "Pausa", "A: seguir", "Editor", "Captura guardada", "Espanol"
  },
  // ---- IT ----
  {
    "Premi A per giocare", "Menu", "Gioca", "Riprendi", "Opzioni",
    "Volume", "Lingua", "Punteggi", "Migliori punti", "Nessun punteggio",
    "Comandi", "Calibra stick", "Schermata", "Torna al loader", "B: indietro",
    "Nome", "A: lettera  C: cancella  B: ok", "Partita finita", "A: rigioca",
    "Pausa", "A: riprendi", "Editor", "Schermata salvata", "Italiano"
  },
};

static Lang s_lang = FR;

const char* T(Str s) {
    if (s >= STR_COUNT) return "?";
    return K[s_lang][s];
}

void set_language(Lang l) { if (l < LANG_COUNT) s_lang = l; }
Lang get_language()       { return s_lang; }
void next_language()      { s_lang = (Lang)((s_lang + 1) % LANG_COUNT); }

} // namespace i18n
