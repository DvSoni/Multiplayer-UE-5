# H.E.V.A

A multiplayer [genre — e.g. third-person shooter / battle royale] built in Unreal Engine
using C++ and Blueprints, focused on networked gameplay systems.

> 🎓 This project began as a learning exercise following Stephen Ulibarri's
> Unreal Engine C++ Developer course on Udemy, and was extended with
> original systems and design choices (see "What I Built" below).

![Gameplay Screenshot](docs/screenshots)


🎥[![Watch the gameplay walkthrough](Linkdin post)](https://www.linkedin.com/posts/divy-soni-20ds089_unrealengine5-unrealengine-epicgames-activity-7097957598536118272-v0DM?utm_source=share&utm_medium=member_desktop&rcm=ACoAADXpfdABO7tlOKoa02gH4e21BRP82jUJJI0)

---

## What I Built

Beyond the course foundation, I designed and implemented the following systems myself:

- **Multiple weapon types** — added a variety of distinct weapons beyond the
  course's base implementation, each with its own behavior/stats.

- **Third-person to first-person camera switching** — implemented a toggle
  between third-person and first-person camera views during gameplay. Using the bluepring to switch the camera form TPP to FPP .

- **Randomized player death animations** — killed players play a randomly
  selected death/kill animation instead of a single fixed one, adding
  variety to combat feedback . And added the randomized logic in the programming so whenever player killed the animation is selected in random form and displayed .
 
- **Custom lobby level** — built a separate, dedicated level for the
  pre-match lobby distinct from the gameplay levels. Every player hang areound for a while before starting the game.
  

## Tech Stack

- **Engine:** Unreal Engine 5
- **Languages:** C++, Blueprints
- **Networking:** [e.g. Unreal's built-in replication / Steam Online Subsystem / EOS]
- **Platform:** [Windows / etc.]


## Project Structure

Source/
  Project_test/
    Private/
      Character/        # Character class implementing core gameplay requirements
      GameMode/          # Game mode logic — resume, pause, exit
      GameState/         # Handles transitions between game states (e.g. waiting, playing, ended)
      PlayerController/  # Weapon switching and player input/control logic
      Weapons/           # Weapon base classes and implementations
      Networking/        # Replication helpers, game mode/session logic
      HUD/               # HUD and menu widgets
Content/
  (large asset packs excluded from repo — see .gitignore)
```

## Running the Project

1. Clone the repo
2. Install Unreal Engine 5
3. Right-click `Project_test.uproject` → Generate Visual Studio project files
4. Open and build in your IDE, or double-click the .uproject to launch the editor

> Note: some large content assets are excluded from this repo to keep it
> portfolio-friendly. [[Link to a Gdrive packaged build](https://drive.google.com/file/d/103Mwlc7iBVNGNF0jXvMxGW1g6wHalUG3/view?usp=sharing) ]

## Credits

- Built on the foundation of Stephen Ulibarri's Unreal Engine C++ Developer
  course (Udemy)
