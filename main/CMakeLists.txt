idf_component_register(
    SRCS
        "app_main.cpp"
        # Core (adaptateurs au-dessus du composant gamebuino)
        "core/audio.cpp"
        "core/input.cpp"
        "core/graphics.cpp"
        "core/sprite_atlas.cpp"
        "core/sdcard.cpp"
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
        "ui"
    # gamebuino DOIT etre dans REQUIRES (pas INCLUDE_DIRS) : c'est le composant
    # materiel qui expose gb_core / gb_graphics / gb_audio / gb_ll_*.
    REQUIRES
        gamebuino
        nvs_flash
        fatfs
        esp_timer
        esp_psram
        spi_flash
        driver
        app_update
        esp_partition
)
