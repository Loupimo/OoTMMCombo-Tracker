# OoTMMCombo-Tracker
An auto-tracker for the OoT-MM combo randomizer ROM. It is a fork of the [multi-client](https://github.com/OoTMM/multi-client) written by Nax and therefore needs his [Project64](https://github.com/OoTMM/Project64-EM/releases/tag/v1.0.3) version with lua script support to work.

# How to use

First you need to be sure that your ROM has been created using the using the coop or multi parameter.

## Playing in singleplayer

Just leave everything in the "Launch" tab as they are and click the "Start Tracking" button.
Then open your project64 with lua script, launch your and game and start the adpater script.
Now you can create a new save and start playing.
Once you have finished to play press the "Stop tracking" button (not mandatory) and click the "Save Tracking" button in order to save the current session state. On your next launch just press the "Load tracking" button to get your progress back.
If you forgot to save your session it is possible to get everything back by doing the first 3 steps. Once all items are collected you can play normally.

If you decide to play in coop later on just read the next section.

## Playing in multiplayer

In the "Launch" tab just check the "Use multiplayer" box and enter the server address and port you want to use. If you don't have your own server just leave the default one.
Then follow the "Playing in singleplayer" section from step 2.

## Side notes

Master Quest is current not supported (same MM JP layouts).
Playing in multi world is not recommended as they is no distinction of which world the collected is coming from.

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
- Parts of this project (all the files under the `Sources/Multi` and `Headers/Multi`) are a fork from the [OOTMM multi-client](https://github.com/OoTMM/multi-client) project.
- The full MIT License is included in the file `MIT-LICENSE.txt`.

## License files
- `LICENSE` → Main license (GPLv3).
- `LGPL-3.0.txt` → Text of the LGPL v3 license (provided with Qt or downloadable [here](https://www.gnu.org/licenses/lgpl-3.0.txt)).