#include "ConfigParams.h"
HashMap* configMapping;

void loadConfigParams(){
    configMapping = createHashMap(ConfigHashing,ConfigCompare);
    //TODO Load them in via a file
    char** ConfigsText = textFileToArray("ConfigSettings/Configs.txt");
    int i = 0;
    while(strcmp(ConfigsText[i],"END OF ARRAY") != 0){
        //create the object
        ConfigParam* configObject = calloc(1,sizeof(ConfigParam));
        //loop till get to new line or end of array
        while(strlen(ConfigsText[i]) > 3 && strcmp(ConfigsText[i],"END OF ARRAY") != 0){
            //grab the data of where the : is in the line
            char *targetPosition = strchr(ConfigsText[i], ':');
            if (targetPosition != NULL) {
                size_t index = targetPosition - ConfigsText[i];
                char* paramName = customSubString(ConfigsText[i], 0,index);
                PRINTFLEVEL2("Finding value for header of %s: ",paramName);
                //go through and see which things was picked up and then fill out that part
                if(strcmp(paramName,"Name") == 0){
                    configObject->name = customSubString(ConfigsText[i],index+2,-1);
                    PRINTFLEVEL2("It was the name %s\n", configObject->name);
                }else if(strcmp(paramName,"Type") == 0){
                    configObject->configType = stringToConfigType(targetPosition + 2);
                     PRINTFLEVEL2("It was a type \n");
                }else if(strcmp(paramName,"Default") == 0){
                    configObject->defaultValue = strtod(targetPosition + 2, NULL);
                    configObject->currentValue = strtod(targetPosition + 2, NULL);
                    PRINTFLEVEL2("It was a default value of %f\n",configObject->defaultValue);
                }else if(strcmp(paramName,"MinValue") == 0){
                    configObject->minValue = strtod(targetPosition + 2, NULL);
                     PRINTFLEVEL2("It was a Min value of %f\n",configObject->minValue);
                }else if(strcmp(paramName,"MaxValue") == 0){
                    configObject->maxValue = strtod(targetPosition + 2, NULL);
                     PRINTFLEVEL2("It was a max value of %f\n",configObject->maxValue);
                }else if(strcmp(paramName,"StepSize") == 0){
                    configObject->stepSize = strtod(targetPosition + 2, NULL);
                     PRINTFLEVEL2("It was a Stepsize of %f\n",configObject->stepSize);
                }else if(strcmp(paramName,"SelectionSize") == 0){
                    configObject->selectionSize = strtod(targetPosition + 2, NULL);
                     PRINTFLEVEL2("It was the selection set size of %i\n",configObject->selectionSize);
                }else if(strcmp(paramName,"SelectionSet") == 0){
                    configObject->selectionSet = convertTocharArray(targetPosition+2,configObject->selectionSize);
                    PRINTFLEVEL2("It was the selection set\n");
                }else{
                    //TODO: throw error that bad config was found
                    PRINTFLEVEL2("Nothing was found for this\n");
                }
                i++;
            }
        }
        //TODO add in a check to make sure that everything is filled out properly
        PRINTFLEVEL2("Software: adding config with name %s to the hash\n",configObject->name);
        insertHashMap(configMapping, (void*) configObject, (void*) (configObject->name));
        PRINTFLEVEL2("the next line of the array is %s\n",ConfigsText[i]);
        if(strcmp(ConfigsText[i],"END OF ARRAY")!= 0){
            i++;
        }
    }
}

const char* configTypeToString(int configType) {

    switch(configType) {
        case ONOFF:     return "ONOFF";
        case NUMERIC:   return "NUMERIC";
        case ONOFFNUMERIC: return "ONOFFNUMERIC";
        case NUMPAD:    return "NUMPAD";
        case OTHER:     return "OTHER";
        default:        return "UNKNOWN";
    }
}

int writeConfigParamsToFile(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return -1;
    }

    void **entries = getAllEntriesHashMap(configMapping);
    if (entries == NULL) {
        fprintf(stderr, "Failed to retrieve config entries\n");
        fclose(fp);
        return 1;
    }
    int total = configMapping->quantity; 

    for (int i = 0; i < total; i++) {
        ConfigParam *cp = (ConfigParam*) entries[i];
        if(!cp) continue;

        fprintf(fp, "Name: %s\n", cp->name);
        fprintf(fp, "Type: %s\n", configTypeToString(cp->configType));
        fprintf(fp, "Default: %f\n", cp->defaultValue);
        fprintf(fp, "MinValue: %f\n", cp->minValue);
        fprintf(fp, "MaxValue: %f\n", cp->maxValue);
        fprintf(fp, "StepSize: %f\n", cp->stepSize);
        fprintf(fp, "SelectionSize: %d\n", cp->selectionSize);
        if (cp->selectionSet && cp->selectionSize > 0)
            fprintf(fp, "SelectionSet: %s\n", cp->selectionSet[0]);
        else
            fprintf(fp, "SelectionSet: \n");

        fprintf(fp, "\n");
    }

    
        fprintf(fp, "END OF ARRAY\n");
        fclose(fp);
        free(entries);
        return 0;
}



ConfigType stringToConfigType(char* str){
    if(strcmp(str,"OnOff") == 0){
        return ONOFF;
    }else if(strcmp(str,"Numeric") == 0){
        return NUMERIC;
    }else if(strcmp(str,"OnOffNumeric") == 0){
        return ONOFFNUMERIC;
    }else if(strcmp(str,"Selection") == 0){
        return SELECTION;
    }else if(strcmp(str,"Other") == 0){
        return OTHER;
    }else{
        return OTHER; //TODO make this into a error
    }
}

//TODO make a test to make sure that this works
char** convertTocharArray(char* str, int size){
    char** listOfEntries = malloc(sizeof(char*) *size);
    char* currentStr = str;
    //loo
    for(int i = 0; i<size;i++){
        char *targetPosition = strchr(currentStr, ',');
        if (targetPosition != NULL) {
            size_t index = targetPosition - currentStr;
            listOfEntries[i] = customSubString(currentStr, 0,index-1);
            currentStr = targetPosition+1;
        }else{
            listOfEntries[i] = strdup(currentStr);
        }
    }
    return listOfEntries;
}


ConfigParam* getConfigByName(char* name){
    return (ConfigParam*) getHashMap(configMapping, name);
}
void freeConfigParam(void* param){
    ConfigParam* temp = (ConfigParam*) param;
    free(temp);
}
void freeCongigFull(){
    destroyHashMap(configMapping,freeConfigParam, StringHashFree);
}
int ConfigHashing(void* key){
    //just adding together the asci values;
    char* st = (char*) key;
    int hash = 0;
    PRINTFLEVEL2("Creating a hash for the string %s\n", st);
    for(int i = 0; i<strlen(st); i++){
        hash += st[i];
    }
    return hash;
}
bool ConfigCompare(void* data, void* key){
    if(strcmp((char*) data, (char*) key) == 0){
        return true;
    }else{
        return false;
    }
}
char** getListOfConfigNames(){
    char** configNames = malloc(sizeof(char*)*configMapping->quantity);
    PRINTFLEVEL2("SOFTWARE: CONFIGS: created the return object\n");
    int j = 0;
    for(int i = 0; i<configMapping->size;i++){
        //problem, not all indexes have stuff in them
        if(configMapping->list[i] != 0){
            ConfigParam* temp = (ConfigParam*) (configMapping->list[i]);
            PRINTFLEVEL2("SOFTWARE: CONFIGS: created the temp object\n");
            configNames[j] = temp->name;
            PRINTFLEVEL2("SOFTWARE: CONFIGS: Populated it with the name %s\n", temp->name);
            j++;
        }
    }
    return configNames;
}
int getLengthOfConfigs(){
    return configMapping->quantity;
}

double* getListOfCurrentValues(){
    double* values = malloc(sizeof(double)*configMapping->quantity);
    int j = 0;
    for(int i = 0; i<configMapping->quantity;i++){
        if(configMapping->list[i] != 0){
        ConfigParam* temp = (ConfigParam*) configMapping->list[i];
        values[j] = temp->currentValue;
        j++;
        }
    }
    return values;

}
void setListOfcurrentValues(double* values){
    int j =0;
    for(int i = 0; i<configMapping->quantity;i++){
         if(configMapping->list[i] != 0){
        ConfigParam* temp = (ConfigParam*) configMapping->list[i];
        temp->currentValue = values[j];
        j++;
         }
    }
}

/**
 * given a config name and true for increment up and false for inclement down
 * returns new value of the setting as a string
*/
char* incrementConfig(char* name, bool up){
    //get the config object
    ConfigParam* param = getHashMap(configMapping,name);
    //incremetn/decrement by the value
    char* str = malloc(sizeof(char)*100);
    switch (param->configType)
    {
    case ONOFF:
        param->currentValue = (param->currentValue*-1)+1;
        if(param->currentValue == 1){
            strcpy(str,"on");
        }else{
           strcpy(str,"off");
        }
        break;
    case NUMERIC:
        param->currentValue += param->stepSize * (up*2 - 1);
        if(param->currentValue > param->maxValue){
            param->currentValue = param->maxValue;
        }else if(param->currentValue < param->minValue){
            param->currentValue = param->minValue;
        }
        sprintf(str, "%f", param->currentValue);
        break;
    case ONOFFNUMERIC:
        break;
    case SELECTION:
        param->currentValue += up*2 - 1;
        if(param->currentValue > param->selectionSize-1){
            param->currentValue = 0;
        }else if(param->currentValue < 0){
            param->currentValue = param->selectionSize -1;
        }
        strcpy(str,param->selectionSet[(int)param->currentValue]);
        break;
    
    default:
    strcpy(str,"Something went wrong");
        break;
    }
    return str;
}

/**
 * directly sets the config value, along with making sure that said value is within the paramiters
 * Returns a string of what the new value is 
*/
char* updateConfigs(char* name, double value){
ConfigParam* param = getHashMap(configMapping,name);
if(param == NULL){
    PRINTFLEVEL1("SOFTWARE: Failed to set value for given config\n");
    return NULL;
}
    //incremetn/decrement by the value
    char* str = malloc(sizeof(char)*100);
    switch (param->configType)
    {
    case ONOFF:
        param->currentValue = value;
        if(param->currentValue == 1){
            strcpy(str,"on");
        }else{
           strcpy(str,"off");
        }
        break;
    case NUMERIC:
    case NUMPAD:
        param->currentValue = value;
        if(param->currentValue > param->maxValue){
            param->currentValue = param->maxValue;
        }else if(param->currentValue < param->minValue){
            param->currentValue = param->minValue;
        }
        sprintf(str, "%f", param->currentValue);
        break;
    case ONOFFNUMERIC:
        break;
    case SELECTION:
        param->currentValue = value;
        if(param->currentValue > param->selectionSize-1){
            param->currentValue = 0;
        }else if(param->currentValue < 0){
            param->currentValue = param->selectionSize -1;
        }
        strcpy(str,param->selectionSet[(int) param->currentValue]);
        break;
    default:
    strcpy(str,"Something went wrong");
        break;
    }
    return str;
}
