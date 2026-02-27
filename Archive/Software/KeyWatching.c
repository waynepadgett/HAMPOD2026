#include "KeyWatching.h"
int keyRequestFrequency =  16000;//was 16000
bool running2 = true;
void* keyWatcher(void* args){
    //TODO properly setup the packet to be sent
    while(running2){
        char pressedKey = readNumPad();
        KeyPress *interpretedKey = interperateKeyPresses(pressedKey);
        //only run the modeFlow iff a key was actualy pressed
        if(interpretedKey->keyPressed != '-'){
            PRINTFLEVEL1("Software: key prKessed %c, shift value %i, was held %i\n", interpretedKey->keyPressed, interpretedKey->shiftAmount,interpretedKey->isHold);
            modeFlow(interpretedKey);
        }
        // else{
        //     // PRINTFLEVEL2("Software: key prKessed %c, shift value %i, was held %i\n", interpretedKey->keyPressed, interpretedKey->shiftAmount,interpretedKey->isHold);
        // }
        free(interpretedKey);
        usleep(keyRequestFrequency);
    }
    return NULL;
}

//These are variables to control the keyPress interperater
bool shiftEnabled = true;
char oldKey = -1;
bool holdKeySent = 0;
int shiftState = 0;
int holdWaitCount = 0;

int maxShifts = 3;
#define keyRequestFrequency2 16000
#define holdSeconds 1
#define holdWaitTime (holdSeconds*1000000) / keyRequestFrequency2

/**
 * Processes the output of the key presses to properly interperate when 
 * a press is shifted or when a press is held down
 * Inputs: key pressed
 * output: a KeyPress struct that holds the data for what key was pressed, this will need to be freed
*/
KeyPress* interperateKeyPresses(char keyPress){
    KeyPress *returnValue = malloc(sizeof(KeyPress));
    returnValue->isHold = false;
    returnValue->keyPressed = '-';
    returnValue->shiftAmount = 0;
    if(keyPress == '-'){
        if(oldKey != '-' && !holdKeySent && holdWaitCount < holdWaitTime){
            if(oldKey == 'A' && getABState()){
                shiftState ++;
                if(shiftState >= maxShifts){
                    shiftState = 0;
                }
            }else{
                returnValue->keyPressed = oldKey;
                returnValue->shiftAmount = shiftState;
                shiftState = 0;
            }//end inner null
        }//end outer if
        holdKeySent = false;
        oldKey = '-';
        holdWaitCount = 0;
    }else if(keyPress == oldKey && keyPress != '-'){
        if(holdWaitCount < holdWaitTime){
            holdWaitCount++;
        }else if(!holdKeySent){
            holdKeySent = true;
            returnValue->keyPressed = oldKey;
            returnValue->shiftAmount = shiftState;
            returnValue->isHold = true;
            shiftState = 0;
        }
    }else{
        oldKey = keyPress;
    }

    if(returnValue->keyPressed == -1){
        returnValue->keyPressed = '-';
    }
    return returnValue;
}

void resetKeyInputVars(){
    shiftEnabled = 1;
    oldKey = -1;
    holdKeySent = false;
    shiftState = 0;
    maxShifts = 3;
    holdWaitCount = 0;
}