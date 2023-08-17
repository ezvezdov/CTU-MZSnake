# Snake game 🐍
This program is a classic snake game on microzed APO board.

This project was made by Yauheni Zviazdou as a semester work of subject Computer Architecture.

[📺 Game video preview 📺](https://youtu.be/0y7wJU27KDQ)

# Installing, compilation and executing
## You need to install arm-linux-gnueabihf-gcc
### Ubuntu / Debian
```bash
sudo apt-get update
sudo apt-get install arm-linux-gnueabihf-gcc
```
### Arch-based
[AUR](https://aur.archlinux.org/packages/arm-linux-gnueabihf-gcc)

## Installing the game
```bash
cd ~
mkdir snake
cd snake
git clone https://github.com/ezvezdov/CTU-MZSnake.git
cd CTU-MZSnake
```

# How to start?

## Connecting with microzed APO
```bash
ssh-add-mzapo-key
make TARGET_IP=192.168.202.xx
```

## Executing
```bash
./snake
```



# Manual
## Menu 
After starting the game, you will see the main menu. There are 2 actions and 3 options.
Navigation in menu:
* Move to next menu - green knob clockwise turn | S key | K key
* Move to previous menu - green knob сounterclockwise | W key | I key.
* Select current option - push green knob | push Space key
* Change text scale - push blue knob
* Exit - Q key
 
### Actions
* New game - start new game.
* Exit - exit programm.
### Options
* Game type [Singleplayer / Multiplayer] - game for 1 player or game for 2 players.
* Speed [Easy / Normal / Hard] - this options set snake speed, Easy - lowest speed, Hard - maximum speed, Normal - normal (in author's meaning) speed of snake.
* Borders [Yes / No] - snake dies if hit into border in Yes option, otherwise snake's head appears in opposite side.
* Eating [Yes / No] - snake can eat itself and opponent snake if Yes selected.

## Game
### Singleplayer gameplay
  Main aim is eat as musch as possible apples (what have black color and respawn at board, when snake eat it). In classic mode snake can eat itself, die when crashes into border.
### Multiplayer gameplay
  Aim in this mode is kill another snake and have bigger scode. Snake die, when it has length of 2 (2 links, jsut head and tail). Snakes both will die if their heads will crashes. Recomended options: turned on eating and turned off borders.
### Red snake controls
* Knobs contol
  * Red knob clockwise - turn snake's head to Right (in snake's view)
  * Red knob сounterclockwise - turn snake's head to
    Left (in snake's view)
* Keyboard control
  * W key - turn snake's head to UP
  * A key - turn snake's head to Left
  * S key - turn snake's head to Down
  * D key - turn snake's head to Right

### Blue snake controls (multilplayer)
* Knobs contol
  * Blue knob clockwise - turn snake's head to Right (in snake's view)
  * Blue knob сounterclockwise - turn snake's head to
    Left (in snake's view)
* Keyboard control
  * I key - turn snake's head to UP
  * J key - turn snake's head to Left
  * K key - turn snake's head to Down
  * L key - turn snake's head to Right

### Other
* Pushing P key you can pause the game (resume by clicking any key)
* Type Q to exit to menu without saving game.

# Project architecture

[UML Diagram](https://gitlab.fel.cvut.cz/B212_B0B35APO/zviazyau/-/blob/main/snake_diagram.pdf)

# Equipment
Description of the hardware of the used MicroZed processor board
MICROZED EVALUATION KIT
* ADSAES-Z7MB-7Z010-G
* Xylinx Zynq 7Z010
* Base Chip: Xilinx Zynq-7000 All Programmable SoC
* Type: Z-7010, part XC7Z010
* CPU: Dual ARM Cortex ™ -A9 MPCore ™ @ 866 (NEON ™ & Single/Double Precision
Floating Point)
* 2x L1 32 kB data + 32 kB instruction, L2 512 KB
* FPGA: 28K Logic Cells (~ 430K ASIC logic gates, 35 kbit)
* Computing units in FPGAs: 100 GMACs
* FPGA memory: 240 KB
* Memory on MicroZed board: 1GB
* Operating system: GNU/Linux
* GNU LIBC (libc6) 2.19-18 + deb8u7
  * Linux kernel 4.9.9-rt6-00002-ge6c7d1c
  * Distribution: Debian Jessie
* More information at http://microzed.org/product/microzed

Interfaces accessible directly on MicroZed board
* 1G ETHERNET,
* USB Host, A connector
* serial port UART1 via converter to USB, USB micro-B
* micro SD card
* on the board is Flash, one user LED, user 
