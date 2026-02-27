#include "NormalMode.h"
int retcode; 

bool enteringValue = false;
char inputValue[100] = ""; 
int setFunctionType = 0; 
HamlibSetFunction currentInputFunction;
setting_t settingToChange; // This goes with Type Two
KeyPress* normalModeClearing; 
 
vfo_t general_vfo = RIG_VFO_CURR;
vfo_t a_vfo = RIG_VFO_A; 
vfo_t b_vfo = RIG_VFO_B; 
vfo_t c_vfo = RIG_VFO_C; 

int switchFuncMode(RIG* radioDetails) {
    int status; 
    retcode = rig_get_func(radioDetails, general_vfo, settingToChange, &status);
    if (retcode == RIG_OK) {
        if (status == 0) {
            printf("Turn func on\n"); 
            return 1; 
        } else {
            printf("Turn func off\n"); 
            return 0; 
        }
    }
    return -1;
}

void enterValueModeTypeOne(KeyPress* keyInput, RIG* radioDetails) {
    double enteredValue = keypadInput(keyInput);
    if (enteredValue >= 0) {
        void** inputArray = malloc(sizeof(void*) * 4); 
        inputArray[0] = radioDetails; 
        inputArray[1] = &general_vfo; 
        inputArray[2] = (void*)(intptr_t)enteredValue; 
        // void** inputArray[] = {(void*) radioDetails, (void*) &general_vfo, (void*) enteredValue};
        char* result = currentInputFunction(inputArray);
        sendSpeakerOutput(result);
        free(result); 
        enteringValue = false; 
        free(inputArray); 
    } else {

    }
}

void enterValueModeTypeTwo(KeyPress* keyInput, RIG* radioDetails) {
    double enteredValue = keypadInput(keyInput);
    printf("%lf\n", enteredValue); 
    if (enteredValue >= 0) {
        void** inputArray = malloc(sizeof(void*) * 4); 
        inputArray[0] = radioDetails; 
        inputArray[1] = &general_vfo; 
        inputArray[2] = (void*)(uintptr_t)settingToChange; 
        inputArray[3] = (void*)(intptr_t)enteredValue; 
        // void** inputArray[] = {(void*) radioDetails, (void*) &general_vfo, (void*) settingToChange, (void*) enteredValue};
        printf("I got here before result\n"); 
        char* result = currentInputFunction(inputArray);
        printf("%s\n", result); 
        sendSpeakerOutput(result);
        free(result); 
        enteringValue = false; 
        free(inputArray); 
    } else {

    }
}

void* normalCommandRelay(KeyPress* keyInput, RIG* radioDetails){
    void** inputArray = malloc(sizeof(void*) * 4); 
    char* result; 
    if (enteringValue == false) {
        switch (keyInput->keyPressed) {
            case '0':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get Noise Reduction: RIG_LEVEL_NR
                            if (rig_has_get_level(radioDetails, RIG_LEVEL_NR)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_NR};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_NR; 

                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Noise Reduction\n"); 
                            }
                            break; 
                        case 1:
                            // Get Compression: RIG_FUNC_COMP
                            if (rig_has_get_func(radioDetails, RIG_FUNC_COMP)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_COMP};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_COMP; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Compression status\n"); 
                            }
                            break; 
                        case 2:
                            // Get Tone Control: RIG_FUNC_TONE
                            if (rig_has_get_func(radioDetails, RIG_FUNC_TONE)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_TONE};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_TONE; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Tone Control status\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set Noise Reduction: RIG_LEVEL_NR
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Noise Reduction\n");
                            settingToChange = RIG_LEVEL_NR; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        case 1:
                            // Set Compression: RIG_FUNC_COMP
                            if (rig_has_set_func(radioDetails, RIG_FUNC_COMP)) {
                                settingToChange = RIG_FUNC_COMP; 
                                int setting = switchFuncMode(radioDetails); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_COMP, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_COMP; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set Compression status\n"); 
                            }
                            break; 
                        case 2:
                            // Set Tone Control: RIG_FUNC_TONE
                            if (rig_has_set_func(radioDetails, RIG_FUNC_TONE)) {
                                settingToChange = RIG_FUNC_TONE; 
                                int setting = switchFuncMode(radioDetails); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_TONE, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_TONE; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set Tone Control status\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                }
                break;
            
            case '1':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get frequency from VFO A
                            // inputArray = {radioDetails, RIG_VFO_A};
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &a_vfo; 
                            result = get_current_frequency(inputArray);
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result);
                            } else {
                                fprintf(stderr, "VFO A frequency error\n");
                            }
                            free(result); // Free the allocated memory
                            break;
                        case 1:
                            // Get frequency from VFO B
                            // inputArray = {radioDetails, RIG_VFO_B};
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &b_vfo; 
                            result = get_current_frequency(inputArray);
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result);
                            } else {
                                fprintf(stderr, "VFO B frequency error\n");
                            }
                            free(result); // Free the allocated memory
                            break;
                        case 2:
                            // Get frequency from VFO C
                            // inputArray = {radioDetails, RIG_VFO_C};
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &c_vfo; 
                            result = get_current_frequency(inputArray);
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result);
                            } else {
                                fprintf(stderr, "VFO C frequency error\n");
                            }
                            free(result); // Free the allocated memory
                            break;
                        default:
                            break;
                    }
                    break; 
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get current VFO
                            // inputArray = {radioDetails};
                            inputArray[0] = radioDetails; 
                            result = get_current_vfo(inputArray); 
                            sendSpeakerOutput(result); 
                            free(result);
                            break; 
                        case 1:
                            // Set current VFO
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &general_vfo; 
                            result = set_vfo_custom(inputArray); 
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result);
                            } else {
                                printf("Cannot set VFO\n"); 
                            }
                            free(result);
                            break; 
                        case 2:
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '2':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get Mode
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &general_vfo; 
                            result = get_current_mode(inputArray); 
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result);
                            } else {
                                printf("Cannot set VFO\n"); 
                            }
                            free(result);
                            break; 
                        case 1:
                            // Get Width
                            break; 
                        case 2:
                            // Getting VFO Lock Status
                            if (rig_has_get_func(radioDetails, RIG_FUNC_LOCK)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_LOCK};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_LOCK; 
                                result = get_level(inputArray); 
                                printf("%s", result); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get VFO lock status\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set Mode
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &general_vfo; 
                            result = set_mode_custom(inputArray); 
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result);
                            } else {
                                printf("Cannot set mode\n"); 
                            }
                            free(result); 
                            break; 
                        case 1:
                            // Set Width
                            break; 
                        case 2:
                            // Setting VFO Lock
                            if (rig_has_set_func(radioDetails, RIG_FUNC_LOCK)) {
                                settingToChange = RIG_FUNC_LOCK; 
                                int setting = switchFuncMode(radioDetails); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_LOCK, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_LOCK; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set VFO lock status\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '3':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get PTT Status
                            // inputArray = {radioDetails, &general_vfo};
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &general_vfo; 
                            result = get_ptt(inputArray);
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result);
                            } else {
                                printf("Cannot get PTT Status\n"); 
                            }
                            free(result); 
                            break;
                        case 1:
                            // Get Passband Tuning Input: RIG_LEVEL_PBT_IN
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_PBT_IN)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_PBT_IN};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_PBT_IN; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Passband Tuning Input\n"); 
                            }
                            break; 
                        case 2:
                            // Get Passband Tuning Output: RIG_LEVEL_PBT_OUT
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_PBT_OUT)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_PBT_OUT};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_PBT_OUT; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get Passband Tuning Output\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set PTT Status
                            // inputArray = {radioDetails, &general_vfo};
                            // result = get_ptt(inputArray);
                            // if (strcmp(result, "-1") != 0) {
                            //     sendSpeakerOutput(result);
                            // } else {
                            //     printf("Cannot set PTT Status\n"); 
                            // }
                            // free(result); 
                            break; 
                        case 1:
                            // Set Passband Tuning Input: RIG_LEVEL_PBT_IN
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Passband Tuning Input\n");
                            settingToChange = RIG_LEVEL_PBT_IN; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        case 2:
                            // Set Passband Tuning Output: RIG_LEVEL_PBT_OUT
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Passband Tuning Output\n");
                            settingToChange = RIG_LEVEL_PBT_OUT; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '4':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get RIT Status (Voice Operated Relay)
                            if (rig_has_get_func(radioDetails, RIG_FUNC_RIT)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_RIT};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_RIT;
                                result = get_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get RIT Status\n"); 
                            }
                            
                            break; 
                        case 1:
                            // Get RIT Offset
                            // inputArray = {radioDetails, &general_vfo};
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &general_vfo; 
                            result = get_current_rit_offset(inputArray); 
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result); 
                            } else {
                                printf("Cannot get RIT Offset\n"); 
                            }
                            free(result); 
                            break; 
                        case 2:
                            // Get Noise Blanker: RIG_FUNC_NB
                            if (rig_has_get_func(radioDetails, RIG_FUNC_NB)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_NB};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_NB; 
                                result = get_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Noise Blanker\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set RIT Status
                            if (rig_has_set_func(radioDetails, RIG_FUNC_RIT)) {
                                settingToChange = RIG_FUNC_RIT; 
                                int setting = switchFuncMode(radioDetails); 
                                printf("%i", setting); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_RIT, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_RIT; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set RIT Status\n"); 
                            }
                            break; 
                        case 1:
                            // Set RIT Offset
                            enteringValue = true;
                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for RIT Offset\n");
                            currentInputFunction = set_rit_offset;
                            setFunctionType = 1; 
                            break; 
                        case 2:
                            // Set Noise Blanker: RIG_FUNC_NB
                            if (rig_has_set_func(radioDetails, RIG_FUNC_NB)) {
                                settingToChange = RIG_FUNC_NB; 
                                int setting = switchFuncMode(radioDetails); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_NB, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_NB; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set Noise Blanker\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '5':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get XIT Status
                            if (rig_has_get_func(radioDetails, RIG_FUNC_XIT)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_XIT};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_XIT; 
                                result = get_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get XIT Status\n"); 
                            }
                            break; 
                        case 1:
                            // Get XIT Offset
                            // inputArray = {radioDetails, &general_vfo};
                            inputArray[0] = radioDetails; 
                            inputArray[1] = &general_vfo; 
                            result = get_current_xit_offset(inputArray); 
                            if (strcmp(result, "-1") != 0) {
                                sendSpeakerOutput(result); 
                            } else {
                                printf("Cannot get XIT Offset\n"); 
                            }
                            free(result); 
                            break; 
                        case 2:
                            // Get Squelch Control: RIG_FUNC_SQL
                            if (rig_has_get_func(radioDetails, RIG_FUNC_SQL)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_SQL};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_SQL; 
                                result = get_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Squelch Control\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set XIT Status
                            if (rig_has_set_func(radioDetails, RIG_FUNC_XIT)) {
                                settingToChange = RIG_FUNC_XIT; 
                                int setting = switchFuncMode(radioDetails); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_XIT, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_XIT; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set XIT Status\n"); 
                            }
                            break; 
                        case 1:
                            // Set XIT Offset
                            enteringValue = true;
                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for XIT Offset\n");
                            currentInputFunction = set_xit_offset;
                            setFunctionType = 1; 
                            break; 
                        case 2:
                            // Set Squelch Control: RIG_FUNC_SQL 
                            if (rig_has_set_func(radioDetails, RIG_FUNC_SQL)) {
                                settingToChange = RIG_FUNC_SQL; 
                                int setting = switchFuncMode(radioDetails); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_SQL, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_SQL; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set Squelch Control\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '6':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get IF Shift: RIG_LEVEL_IF
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_IF)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_IF};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = (void*) &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_IF; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get Squelch\n"); 
                            }
                            break; 
                        case 1:
                            // Get Automatic Gain Control: RIG_LEVEL_AGC
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_AGC)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_AGC};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_AGC; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get Automatic Gain Control\n"); 
                            }
                            break; 
                        case 2:
                            // Get Squelch: RIG_LEVEL_SQL
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_SQL)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_SQL};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_SQL; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get Squelch\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set IF Shift: RIG_LEVEL_IF
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for IF Shift\n");
                            settingToChange = RIG_LEVEL_IF; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            keypadInput(normalModeClearing); // Clear previous input
                            break;
                        case 1:
                            // Set Automatic Gain Control: RIG_LEVEL_AGC
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Automatic Gain Control\n");
                            settingToChange = RIG_LEVEL_AGC; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        case 2:
                            // Set Squelch: RIG_LEVEL_SQL
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Squelch\n");
                            settingToChange = RIG_LEVEL_SQL; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '7':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get VOX Status (Voice Operated Relay): RIG_FUNC_VOX
                            if (rig_has_get_func(radioDetails, RIG_FUNC_VOX)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_VOX};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_VOX; 
                                result = get_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get VOX Status\n"); 
                            }
                            break; 
                        case 1:
                            // Get Audio Frequency (AF): RIG_LEVEL_AF
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_AF)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_AF};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_AF; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get Audio Frequency\n"); 
                            }
                            break; 
                        case 2:
                            // Get Radio Frequency (RF): RIG_LEVEL_RF
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_RF)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_RF};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_RF; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get Radio Frequency\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set VOX Status (Voice Operated Relay): RIG_FUNC_VOX
                            if (rig_has_set_func(radioDetails, RIG_FUNC_VOX)) {
                                settingToChange = RIG_FUNC_VOX; 
                                int setting = switchFuncMode(radioDetails); 
                                // inputArray = {radioDetails, &general_vfo, RIG_FUNC_VOX, setting}; 
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_FUNC_VOX; 
                                inputArray[3] = (void*)(uintptr_t)setting; 
                                result = set_func(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot set VOX Status\n"); 
                            }
                            break; 
                        case 1:
                            // Set Audio Frequency (AF): RIG_LEVEL_AF
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Audio Frequency\n");
                            settingToChange = RIG_LEVEL_AF; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        case 2:
                            // Set Radio Frequency (RF): RIG_LEVEL_RF
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Radio Frequency\n");
                            settingToChange = RIG_LEVEL_RF; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '8':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get VOX Gain: RIG_LEVEL_VOXGAIN
                            if (rig_has_get_level(radioDetails, RIG_LEVEL_VOXGAIN)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_VOXGAIN};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_VOXGAIN; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get VOX Gain\n"); 
                            }
                            break; 
                        case 1:
                            // Get Microphone Gain: RIG_LEVEL_MICGAIN
                            if (rig_has_get_level(radioDetails, RIG_LEVEL_MICGAIN)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_MICGAIN};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_MICGAIN; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Microphone Gain\n"); 
                            }
                            break; 
                        case 2:
                            // Get RF Power: RIG_LEVEL_RFPOWER
                            if (rig_has_get_level(radioDetails, RIG_LEVEL_RFPOWER)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_RFPOWER};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_RFPOWER; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get RF Power\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set VOX Gain: RIG_LEVEL_VOXGAIN
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for VOX Gain\n");
                            settingToChange = RIG_LEVEL_VOXGAIN; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        case 1:
                            // Set Microphone Gain: RIG_LEVEL_MICGAIN
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Microphone Gain\n");
                            settingToChange = RIG_LEVEL_MICGAIN; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        case 2:
                            // Set RF Power: RIG_LEVEL_RFPOWER
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for RF Power\n");
                            settingToChange = RIG_LEVEL_RFPOWER; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        default:
                            break;
                    }
                }
                break;
            case '9':
                if (!keyInput->isHold) {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Get VOX Delay: RIG_LEVEL_VOXDELAY
                            if (rig_has_get_level(radioDetails, RIG_LEVEL_VOXDELAY)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_VOXDELAY};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_VOXDELAY; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                                
                            } else {
                                printf("Cannot get VOX Delay\n"); 
                            }
                            break; 
                        case 1:
                            // Get Audio Balance: RIG_LEVEL_BALANCE
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_BALANCE)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_BALANCE};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_BALANCE; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Audio Balance\n"); 
                            }
                            break; 
                        case 2:
                            // Get Standing Wave Ratio: RIG_LEVEL_SWR
                            if (rig_has_get_func(radioDetails, RIG_LEVEL_SWR)) {
                                // inputArray = {radioDetails, &general_vfo, RIG_LEVEL_SWR};
                                inputArray[0] = radioDetails; 
                                inputArray[1] = &general_vfo; 
                                inputArray[2] = (void*)(uintptr_t)RIG_LEVEL_SWR; 
                                result = get_level(inputArray); 
                                sendSpeakerOutput(result); 
                                free(result); 
                            } else {
                                printf("Cannot get Standing Wave Ratio\n"); 
                            }
                            break; 
                        default:
                            break;
                    }
                } else {
                    switch (keyInput->shiftAmount) {
                        case 0:
                            // Set VOX Delay: RIG_LEVEL_VOXDELAY
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for VOX Delay\n");
                            settingToChange = RIG_LEVEL_VOXDELAY; 
                            currentInputFunction = set_level;
                            int setFunctionType = 2; 
                            break; 
                        case 1:
                            // Set Audio Balance: RIG_LEVEL_BALANCE
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Audio Balance\n");
                            settingToChange = RIG_LEVEL_BALANCE; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        case 2:
                            // Set Standing Wave Ratio: RIG_LEVEL_SWR
                            enteringValue = true;

                            memset(inputValue, 0, sizeof(inputValue)); 
                            sendSpeakerOutput("Enter value for Standing Wave Ratio\n");
                            settingToChange = RIG_LEVEL_SWR; 
                            currentInputFunction = set_level;
                            setFunctionType = 2; 
                            break; 
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }else{
        switch(setFunctionType) {
            case 1: 
                enterValueModeTypeOne(keyInput, radioDetails);
                break; 
            case 2: 
                enterValueModeTypeTwo(keyInput, radioDetails);
                break; 
            case 3: 
                // enterValueMode(keyInput, radioDetails, currentInputFunctionTypeThree);
                break; 
            default: 
                break; 
        }
    }
    free(inputArray); 
    return NULL;
}


void freeNormalMode(Mode* modeToFree){
    free(modeToFree->modeDetails->modeName);
    free(modeToFree->modeDetails);
    free(modeToFree);
}

Mode* NormalLoad(){
    Mode* newMode = malloc(sizeof(Mode));
    normalModeClearing = malloc(sizeof(KeyPress)); 
    normalModeClearing->keyPressed = '#'; 

    if(newMode == NULL){
        return NULL;
    }
    newMode->modeInput = normalCommandRelay;
    newMode->freeMode = freeNormalMode;
    newMode->enterMode = NULL;
    newMode->exitMode = NULL;

    ModeData* newData = (ModeData*)malloc(sizeof(ModeData));

    if(newData == NULL){
        free(newMode);
        return NULL;
    }
    newData->modeName = strdup("Normal");
    newData->radioModel = 42;

    newMode->modeDetails = newData;

    return newMode;
}