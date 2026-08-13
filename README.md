# PacCore

This is a Pac-Man remake made with C++ and raylib. It has a random maze, ghost AI, and a small extra feature called EMP, which stuns the ghosts for a few seconds.

## Project structure

```
PacCore/
├─ CMakeLists.txt
├─ README.md
├─ assets/
│  ├─ audio/          chomp.mp3, death.mp3, siren.mp3
│  └─ data/           highscore.txt (made and updated when you play)
├─ include/
│  ├─ core/            game state, constants, maze, main game loop
│  └─ entities/        pacman.h, ghost.h
├─ src/
│  ├─ core/             one .cpp file for each header in core/
│  ├─ entities/          one .cpp file for each header in entities/
│  └─ main.cpp
└─ build/               made by CMake when you build the project
```

The code is split into two folders. `core` holds the parts that run the game itself: the maze, the constants, the game loop. `entities` holds the two characters, Pac-Man and the ghosts.

The game logic lives inside a `Game` class, defined in `game.h` and `game.cpp`. `main.cpp` just creates a `Game` object and runs it.

## What you need

- CMake, version 3.11 or newer
- A compiler that supports C++17
- raylib. CMake will try to find it on your computer first. If it can't, it will download and build raylib for you (you need internet for this part).

## How to build and run

```bash
cd /c/PacCore && cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && cmake --build build && ./build/PacCore.exe
```

The `-DCMAKE_POLICY_VERSION_MINIMUM=3.5` flag avoids CMake policy errors that can come up when raylib is fetched and built from source. Adjust `/c/PacCore` to wherever you've cloned the project.

On Windows the executable might be somewhere like `build\Debug\PacCore.exe`, depending on which compiler you use.

After building, CMake copies the `assets` folder next to the executable, so the sounds and the high score file are found no matter where you run it from.

## Controls

- Move: arrow keys or W/A/S/D
- EMP (stuns ghosts for 3 seconds, 10 second cooldown): Space
- Play again after winning or losing: R
- Quit after winning or losing: Q