#pragma once
#include <cstdint>

constexpr int powerup_w = 12;
constexpr int powerup_h = 12;

constexpr int powerups_sheet_w = 48;   // 4 colonnes × powerup_w
constexpr int powerups_sheet_h = 108;  // 9 lignes × powerup_h
constexpr int powerups_sheet_size = powerups_sheet_w * powerups_sheet_h;

extern const uint16_t powerups_pixels[powerups_sheet_size];