# AKAsseBricks sur PC via le backend SDL officiel (jmp42)

Fait tourner **le vrai code AKA** d'AKAsseBricks sur PC : le backend
`Gamebuino_AKA_lib_sdl` recompile le `gb_lib` embarque dans `components/gamebuino`
et remplace la couche `gb_ll_*` (ecran, audio, SD, boutons) par SDL2. Rendu,
mixeur 44100 Hz mono, police, taches : identiques a la console.

## Prerequis (MSYS2 MinGW64)

```
pacman -S mingw-w64-x86_64-{gcc,cmake,ninja,SDL2}
```

Recuperer le backend, p.ex. dans `C:/github/Gamebuino_AKA_lib_sdl`.

## Compiler

Depuis le dossier du jeu (celui qui contient `main/`) :

```
cmake -S sdl -B sdl/build -G Ninja -DAKA_SDL_BACKEND=C:/github/Gamebuino_AKA_lib_sdl
cmake --build sdl/build
```

Produit `sdl/build/akassebricks.exe` (ou `akassebricks` sous Linux/macOS).
Aucun `main` a ecrire : le backend appelle `app_main`.

## La carte SD = un dossier

Le jeu lit/ecrit sous `/sdcard` (reglages, meilleurs scores dans
`/sdcard/AKAsseBricks/`). On pointe cette racine vers un dossier hote :

```
# Windows (cmd)
set GB_SDL_SDCARD=%CD%\sdcard
# Linux / macOS
export GB_SDL_SDCARD="$PWD/sdcard"
```

Le dossier et ses sous-dossiers sont crees a la premiere ecriture. Sans variable,
le backend cree un dossier `sdcard/` a cote de l'executable.

## Lancer / options

```
sdl/build/akassebricks        # ou .exe
```

`GB_SDL_SCALE=4` (fenetre plus grande), `GB_SDL_MUTE=1` (silence),
`GB_SDL_EXIT_AFTER=5` (quitte apres 5 s). Les touches sont affichees au
demarrage ; **Echap = RUN** (extinction => fin du programme).
