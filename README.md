# OoTMMCombo-Tracker

An auto-tracker for the [OoTMM combo](https://ootmm.com/) randomizer. It is a fork of the [multi-client](https://github.com/OoTMM/multi-client) written by Nax and therefore needs his [Project64](https://github.com/OoTMM/Project64-EM/releases/tag/v1.0.3) version with lua script support to work.
This app automatically tracks which object has been collected and update it with the item it contains (just as you would do using the spoiler log but in a more efficient, graphical and user-friendly manner)

# How to use

First you need to be sure that your ROM has been created using the coop or multi world parameter.

## Playing in singleplayer

1. In the "Launch" tab click the "Start Tracking" button.
2. Then open your project64 with lua script, launch your and game and start the adpater script.
3. Now you can create a new save and start playing.

There is an auto-saving feature that is enabled by default. When an item is collected the progress will be saved.\
You can also save your progress manually.

## Playing in multiplayer

In the "Launch" tab just check the "Use multiplayer" box and enter the server address and port you want to use.\
If you don't have your own server just leave the default one.\
Then follow the "Playing in singleplayer" section.

## Side notes

The program is not reading game memory. Therefore, it can't retreive items that have been collected without using the tracker.\
Master Quest is currently not supported (same for MM JP layouts).\
Playing in multi world is not recommended as there is no distinction of which world the collected item is coming from.

# 📜 Licenses used in this project

## Main license
This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for more information.

## Qt and the LGPL
This project uses the **Qt** library, which is distributed under the **LGPL v3** license.
Under the terms of the LGPL, users are allowed to **replace or modify** the Qt libraries used.

The Qt libraries can be obtained from the official website:
[https://www.qt.io/download](https://www.qt.io/download)

### LGPL obligations:
- The Qt libraries used in this application are **dynamically linked**.
- The user may replace these libraries with another compatible version.
- The LGPL v3 license is included in the file `LGPL-3.0.txt`.

### MIT obligations:
Some of the code in this project comes from software under **MIT License**.
- Parts of this project (all files under the `Sources/Multi` and `Headers/Multi` folders) are a fork from the [OOTMM multi-client](https://github.com/OoTMM/multi-client) project.
- The full MIT License is included in the file `MIT-LICENSE.txt`.

## License files
- `LICENSE` → Main license (GPLv3).
- `LGPL-3.0.txt` → Text of the LGPL v3 license (provided with Qt or downloadable [here](https://www.gnu.org/licenses/lgpl-3.0.txt)).
- `MIT-LICENSE.txt` → Multi-Client fork.
