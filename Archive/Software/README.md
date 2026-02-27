# Hampod Software #

  

## Description ##

  

This section contains all of the code related to the Software. In the scope of this repo, Talking to the firmware, keeping track of the current state/mode of the hampod, Using hamlib to control the radio, holding the modes of the radio, holding the configs of the hampod.


### Key press objects
The hampod uses a struct called `KeyPress` to represent keypresses in the Software

`KeyPress` have 3 components to them
* `keyPressed` A charictors representing which key was pressed
* `shiftAmount` How many times the shift key (A) was pressed before the key was pressed
* `isHold` If the key was held down or not
  

### Modes folder ###

  

The modes and loading up the modes is handled here

  

* ModeRouting

  

Loads in the modes and determins which modes are active in the current version of the hampod.

  

To add a new mode to ModeRouting

  
0) In `ModeRounting.c`
1) Increment `modeCount` by 1

2) add in the following code
	`case N:`
	`   if(modes[N] == 0){`
	`modes[N] = modeLoad();`
	`}`
	`return modes[N];`
	`break;`
	Replace N with the new number for `modeCount`
	Replace the `modeLoad()` with the corisponding load function for the mode you want to add
	3) in `ModeRouting.h` add
	`#include  "ModeFileName.h"`

* Modes
Each mode has the following components
* `Mode struct` This is the object that will hold all of the data about the mode
* `ModeData struct` This hold additinal information about the modes like it name and is held in the `Mode struct`
* `createMode()` Creates the mode object as well as load up any additinal information for said mode that would need to be loaded in and is placed within the modes .c file
* `free	Mode(Mode**  modeToFree)` Frees the mode object and is held in the `Mode struct`
* `CommandRelay(KeyPress*  keyInput, int  radioDetails)` This is the main part of the mode that is handed what key on the hampod was pressed and the nessesary information about the radio being used in order to use hamlib. This is held within the `Mode struct`

### Config Settings Folder ###