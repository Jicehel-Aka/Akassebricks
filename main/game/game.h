#pragma once
#include "ball.h"
#include "paddle.h"
#include "brick.h"
#include "powerup.h"
#include "input.h" 
#include "level.h"
#include <vector>

struct Paddle;
struct Ball;
struct PowerUp;

struct Projectile {
    float x, y;       // position
    float vx, vy;     // vitesse
    int w = 2;        // largeur du projectile
    int h = 6;        // hauteur du projectile
    bool active = true;
};

struct GameState {
    enum class State {
        Title,      // écran de titre
        Playing,    // partie en cours
        Paused,     // partie en pause volontaire
        WaitingBall,// attente relance balle
        GameOver,   // partie terminée
        Editor,     // éditeur de niveaux
        Highscores, // affichage des scores
        Options     // menu de réglage des options (pour le son actuellement)
    };
    State state;
    Level level;                   // données du niveau courant
    Paddle paddle;
    std::vector<Ball> balls;
    std::vector<PowerUp> falling;
    std::vector<Projectile> shots;
    int score = 0;
    int lives = 3;
    int levelIndex = 0;            // numéro du niveau courant
};

// Fonctions principales
void spawn_sticky_ball(GameState& g);
void launch_ball(GameState& g);
void game_init(GameState& g);
void game_update(GameState& g);
void game_draw(const GameState& g, bool present = true);
bool game_is_over(const GameState& g);

// Collisions robustes (avec positions précédentes)
bool collision_powerup_paddle(const PowerUp& p, const Paddle& pad, const PowerUp& oldP);
bool collision_ball_brick(const Ball& b, const Brick& brick, const Ball& oldBall);
bool collision_projectile_brick(const Projectile& s, const Brick& brick, const Projectile& oldShot);
bool collision_with_paddle(const Ball& b, const Paddle& p, const Ball& oldBall);

// Génération aléatoire de powerup
PowerType choose_random_powerup();