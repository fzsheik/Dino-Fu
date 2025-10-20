# Dino Fu 🦖🥊

A fast-paced multiplayer platformer fighting game inspired by Boomerang Fu, featuring dinosaurs battling it out with boomerangs!

<img width="771" height="496" alt="image" src="https://github.com/user-attachments/assets/6985d783-d8f0-4931-b86d-e88fe0db66fc" />

## 🎮 About

Dino Fu is a competitive local multiplayer game where players control dinosaurs in arena-style combat. Originally inspired by the Nintendo game Boomerang Fu, this project evolved into a unique platformer experience with smooth movement mechanics and strategic boomerang combat.

Project Goals
This project was created to explore game development from scratch without relying on third-party engines like Unity or Godot. The challenge was to build everything ground-up, including:
- Custom physics engine - Designed specifically to handle boomerang mechanics, including ricocheting off walls and return trajectories
- From-scratch implementation - All game systems built without engine assistance
  
I've always admired how Boomerang Fu manages to be simple yet incredibly deep in its gameplay. To tackle this complexity in a more manageable way, I reimagined it as a platformer - - maintaining the core boomerang combat mechanics while simplifying the game structure.

**Note:** This is a playable prototype with some mechanical and UI bugs still present.

## ✨ Features

- Local multiplayer support (2+ players)
- Smooth platformer movement mechanics
- Strategic boomerang combat with wall bouncing
- Controller-based gameplay
- Arena-style combat

## 🎯 How to Play

### Controls (PlayStation Controller)

- **Left Joystick** - Move your character
- **X Button** - Jump
- **Square Button** - Dash
- **R1** - Throw boomerang
- **R1 (Hold) + Right Joystick** - Aim and throw boomerang in specific direction

### Keyboard (Menu Navigation Only)
- **Enter** - Start game / Restart after match

### Game Rules

- Goal: Eliminate other players to win
- You can only throw one boomerang at a time
- Boomerangs bounce off walls - use the environment strategically!
- Last dino standing wins

![Arena Screenshot](placeholder-arena.png)

## 🚀 Getting Started

### Prerequisites

Make sure you have:
- C++ compiler (g++)
- Required libraries and dependencies (see config files)
- Game controller (required for gameplay)

### Installation & Compilation

1. Clone the repository
```bash
git clone [your-repo-url]
cd dino-fu
```

2. Verify configuration files
   - Check that all include paths are correct
   - Ensure library versions match your system
   - Verify all necessary files are present

3. Compile from `runner.cpp`
```bash
g++ runner.cpp -o runner [your-compiler-flags]
```

4. Run the game
```bash
./runner
```

## 🎮 Player Setup

1. Connect controllers before launching the game
2. Use controllers to join the game (minimum 2 players required)
3. Press **Enter** on keyboard to start the match
4. After match ends, press **Enter** to restart or quit

**Important:** Keyboard is only for menu navigation - controllers are required for actual gameplay!

## 🐛 Known Issues

- Some random mechanical bugs may occur during gameplay
- UI bugs present in certain screens

## 🛠️ Technical Details

- **Main Entry Point:** `runner.cpp`
- **Language:** C++
- **Input:** Controller-based (keyboard for menus only)

## 📝 Future Improvements

- Fix existing mechanical and UI bugs
- Add keyboard support for gameplay
- Implement more arenas
- Add power-ups and different boomerang types
- Single-player mode with AI opponents
---

**Have fun and may the best dino win!** 🦖⚔️
