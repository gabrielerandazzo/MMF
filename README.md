## MMF - Mac Move Focus
MMF is a small macOS utility that lets you switch focus between monitors using only the keyboard. 

## Why I Built This
Working with multiple monitors, I constantly switch between tasks: writing code on one screen, checking docs or videos on another.
As vim user I got tired of interrupting my rhythm just to move the mouse across displays.

## What It Does
- Detects all active displays
- Determines which display your mouse is currently on
- Moves the cursor to the center of the next monitor
- Identifies the topmost valid window on that display
- Brings that window to focus using Accessibility APIs

## Requirements
This tool is meant to be used with <strong>[skhd](https://github.com/koekeishiya/skhd)</strong>, a lightweight hotkey daemon that allows you to bind MMF to whatever keyboard shortcut you prefer.

## Installation
Clone the repository:
```
git clone https://github.com/gabrielerandazzo/MMF.git mmf
cd mmf
```
Make the setup script executable and run it:
```
chmod +x setup.sh
./setup.sh
```
This will compile the tool and place the mmf binary in:
```
~/.config/mmf/mmf
```

## Usage with skhd
Edit your skhd configuration file:
```
~/.config/skhd/skhdrc
```
Add a shortcut of your choice Eg.:
```
ctrl - m: ~/.config/mmf/mmf
```
Reload skhd:
```
skhd -restart
```
## Contributing

Contributions are welcome. If you’d like to improve MMF, fix a bug, or add a new feature, feel free to open an issue or submit a pull request.
