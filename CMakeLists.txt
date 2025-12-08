idf_component_register(
    SRCS
        "app_main.cpp"
        # Lib / matériel
        "lib/LCD.c"
        "lib/expander.c"        
        "lib/audio.c"   
        "lib/audio_basic.cpp"
        "lib/graphics_basic.cpp"  
        "lib/sdcard.c"
        # Core
        "core/audio.cpp"
        "core/input.cpp"
        "core/graphics.cpp"
        "core/sprite_atlas.cpp"
        "core/persist.cpp"      
        # Assets
        "assets/assets.cpp"
        "assets/spritesheet_bricks.cpp"
        "assets/spritesheet_powerups.cpp"
	"assets/assets_paddle.cpp"
        "assets/spritesheet_paddle.cpp"
        "assets/title_image.cpp"
        # Game
        "game/config.cpp"
        "game/game.cpp"
        "game/paddle.cpp"
        "game/ball.cpp"
        "game/brick.cpp"
        "game/powerup.cpp"
        "game/level.cpp"
        "game/level_editor.cpp"
        # UI
        "ui/highscores.cpp"
        "ui/title_screen.cpp"
        "ui/menu.cpp"
        "ui/hud.cpp"
    INCLUDE_DIRS
        "."
        "core"
        "game"
        "assets"
        "lib"
        "ui"
)