#include <stdio.h>
#include "ConfigLoad.h"
bool loadUpFromSaveOLD(int saveFileNumber){
    //get the file to load
    char fileName[100];
    sprintf(fileName, "ConfigSettings/SaveFiles/saveNumber%i.txt",saveFileNumber);
    PRINTFLEVEL1("SOFTWARE: loading up save file %s\n",fileName);
    char** saveFile = textFileToArray(fileName);

    if( (int) saveFile == -1){
        PRINTFLEVEL1("SOFTWARE: Failed to load up save file %s\n",fileName);
        return false;
    }
    PRINTFLEVEL1("SOFTWARE: loaded up the file %s\n", fileName);
    int i = 0;
    printf("%s\n",saveFile[i]);
    i++;
    while(strcmp(saveFile[i], "Start of Hotkeys") != 0){
        /*
        1) substring to get the components
        2) convert the 2nd part to a double
        3) grab the config object
        4) set the value to the found double
        */
        // 1
        char* doubleString = strchr(saveFile[i], ':');
        int index = (int)(doubleString - saveFile[i]);
        char* Name = customSubString(saveFile[i],0, index);
        // 2
        double value = atof(doubleString+1);
        //3 4 

        PRINTFLEVEL1("SOFTWARE: loading value %f for config %s\n",value,Name);
        updateConfigs(Name, value);
        i++;
    }
    printf("%s\n",saveFile[i]);
    i++;
    while(strcmp(saveFile[i], "Start of radios") != 0){
        /*
        1) Convert substring into index and Name
        2) See if the name not null
        3) set the hotkey via getting the mode by name
        */
       //TODO set this up
        char* nameString = strchr(saveFile[i], ':') + 1;
        int seporatorIndex = (int)(nameString - saveFile[i]);
        int index = atoi(customSubString(saveFile[i],0, seporatorIndex));
        //2
        if(strcmp("NULL", nameString) != 0){
            PRINTFLEVEL1("Loading up mode %s into index %i\n",nameString,index);
            setProgramibleKeysByIndex(index,nameString);
        }
        i++;
    }
    printf("%s\n",saveFile[i]);
    i++;
    int j = 0;
    while(strcmp(saveFile[i], "END OF ARRAY") != 0){
        /*
        1) substring to get the components
        2) get the hampod ID and the port name
        3) link them together
        4) save it to the thing
        */
        char* point0 = strchr(saveFile[i], ':');
        char* point1 = strchr(point0+1, ':');
        char* point2 = strchr(point1+1, ':');
        PRINTFLEVEL2("SOFTWARE: Points found are %s,%s,%s\n",point0,point1,point2);
        int dis0 = (int) (point0 - saveFile[i]);
        int dis1 = (int) (point1 - point0);
        int dis2 = (int) (point2 - point1);
        int dis3 =  strlen(point2);
        PRINTFLEVEL2("SOFTWAR: the lengths are %i,%i,%i,%i\n",dis0,dis1,dis2,dis3);
        char* sMake = customSubString(saveFile[i],0,dis0);
        char* sModel = customSubString(saveFile[i],point0 - saveFile[i]+1,dis1);
        char* sPort = customSubString(saveFile[i],point1 - saveFile[i]+1,dis2);
        char* sRigModel =customSubString(saveFile[i],point2 - saveFile[i]+1,dis3);
        PRINTFLEVEL2("SOFTWARE: Created filtered strings %s, %s, %s, %s to load in\n",sMake,sModel,sPort,sRigModel);
        int iModel = atoi(sModel);
        int iPort = atoi(sPort);
        int iRigModel = atoi(sRigModel);
        PRINTFLEVEL1("Loading up radio id %i\n",iRigModel);
        loadUpRadioUsingData(sMake,iModel,iPort,getModeByName("Normal"),iRigModel);
        j++;
        i++;
    }
    free(saveFile);
    return true;
}

/**
 * Takes in a key press
 * Returns 0 when no selection has been made
 * Returns 1 when a file is loaded up
*/
int loadSaveBootup(KeyPress* keyinput){
    switch (keyinput->keyPressed)
    {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case '*':
    case '#':
        PRINTFLEVEL1("Software: Invalid save file chosen\n");
        return false;
        break;
    default:
        if(loadUpFromSaveOLD(convertCharToKeyValue(keyinput))){
            return 1;
        }else{
            return 0;
        }
        break;
    }
    return 0;
}
