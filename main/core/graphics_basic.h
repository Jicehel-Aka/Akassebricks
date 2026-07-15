/*
  core/graphics_basic.h — Compat : l'ancienne classe graphics_basic est
  desormais gb_graphics (meme API : setColor, fillRect, drawRect, drawLine,
  fillCircle...). Un simple alias suffit, le jeu n'a rien a changer.
*/
#pragma once
#include "gb_graphics.h"
using graphics_basic = gb_graphics;
