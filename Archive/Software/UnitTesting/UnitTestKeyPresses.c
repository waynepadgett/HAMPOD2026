bool compareKeyInputs(KeyPress* correct, KeyPress* testing){
    if(correct->keyPressed == testing->keyPressed && correct->shiftAmount == testing->shiftAmount && correct->isHold == testing->isHold){
        return true;
    }else{
        return false;
    }
}


void printOutErrorForWhatWasExpected(KeyPress* exepected, KeyPress* actual, char eo, bool ehks, int ess, int ehc){
    printf("keyPressed Expected:%c Actual:%c\nShiftAmount Expected:%i Actual:%i\nisHold Expected:%i Actual:%i\n",exepected->keyPressed,actual->keyPressed,exepected->shiftAmount,actual->shiftAmount,exepected->isHold,actual->isHold);
    printOutErrors(eo,ehks,ess,ehc);
}
/**
 * the idea for this is to have an array of the values to just itterate though so there is no need to have too many lines of code
*/
bool fullTest(){
    resetKeyInputVars();
    KeyPress* exepected = malloc(sizeof(KeyPress));
    KeyPress* actual;

    //this is for the actual return
    char input[] =              {-1 ,'B',-1 ,-1 ,'A',-1 ,'C',-1 ,'A',-1 ,'A',-1 ,-1 ,'D','D','D','D',-1 ,'A',-1 ,'A','A','A','A',-1 };
    char expectedKeyPressed[] = {-1 ,-1 ,'B',-1 ,-1 ,-1 ,-1 ,'C',-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,'D',-1 ,-1 ,-1 ,-1 ,-1 ,-1 ,'A',-1};
    int expectedShiftAmount[] = {0  ,0  ,0  ,0  ,0  ,0  ,0  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,2  ,0  ,0  ,0  ,0  ,0  ,0  ,1  ,0};
    bool expectedifHold[] =     {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,true,false};

    char expectedOld[] =        {-1 ,'B',-1 ,-1 ,'A',-1 ,'C',-1 ,'A',-1 ,'A',-1 ,-1 ,'D','D','D','D',-1 ,'A',-1 ,'A','A','A','A',-1 }; 
    bool expectedHoldKeySent[] ={false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,true,false};
    int expectedShiftState[] =  {0  ,0  ,0  ,0  ,0  ,1  ,1  ,0  ,0  ,1  ,1  ,2  ,2  ,2  ,2  ,2  ,0  ,0  ,0  ,1  ,1  ,1  ,1  ,0  ,0};
    int expectedHoldCount[] =   {0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,1  ,2  ,2  ,0  ,0  ,0  ,0  ,1  ,2  ,2  ,0};
    int trialLength = 25;
    int i;
    for(i = 0; i<trialLength; i++){
        //printf("Current input %c, input number %i out of %i\n", input[i],i,trialLength);
        actual = interperateKeyPresses(input[i]);
        exepected->keyPressed = expectedKeyPressed[i];
        exepected->shiftAmount = expectedShiftAmount[i];
        exepected->isHold = expectedifHold[i];
       // printOutErrorForWhatWasExpected(exepected, actual,expectedOld[i],expectedHoldKeySent[i],expectedShiftState[i],expectedHoldCount[i]);
        if(!compareKeyInputs(exepected,actual)){
            //printOutErrorForWhatWasExpected(exepected, actual,expectedOld[i],expectedHoldKeySent[i],expectedShiftState[i],expectedHoldCount[i]);
            free(actual);
            free(exepected);
            return false;
        }
        if(!confirmKeyInputVars(expectedOld[i],expectedHoldKeySent[i],expectedShiftState[i],expectedHoldCount[i])){
            //printOutErrorForWhatWasExpected(exepected, actual,expectedOld[i],expectedHoldKeySent[i],expectedShiftState[i],expectedHoldCount[i]);
            free(actual);
            free(exepected);
            return false;
        }
        free(actual);
    }
    free(exepected);

    return true;
}

bool testKeyPadEntering(){
    KeyPress* testInput = malloc(sizeof(KeyPress*));
    double returnValue = 0;
    testInput->keyPressed = '1';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-1.0,returnValue);
    if(returnValue != -1.0){
        return false;
    }
    testInput->keyPressed = '2';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-12.0,returnValue);
    if(returnValue != -12.0){
        return false;
    }
    testInput->keyPressed = '3';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-123.0,returnValue);
    if(returnValue != -123.0){
        return false;
    }
    testInput->keyPressed = '4';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-1234.0,returnValue);
    if(returnValue != -1234.0){
        return false;
    }
    testInput->keyPressed = '#';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",1234.0,returnValue);
    if(returnValue != 1234.0){
        return false;
    }
    testInput->keyPressed = '5';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-5.0,returnValue);
    if(returnValue != -5.0){
        return false;
    }
    testInput->keyPressed = '6';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-56.0,returnValue);
    if(returnValue != -56.0){
        return false;
    }
    testInput->keyPressed = '*';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-56.0,returnValue);
    if(returnValue != -56.0){
        return false;
    }
    testInput->keyPressed = '7';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-56.7,returnValue);
    if(returnValue != -56.7){
        return false;
    }
    testInput->keyPressed = '8';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-56.78,returnValue);
    // if(returnValue != -56.78){
    //     return false;
    // }
    testInput->keyPressed = '#';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",56.78,returnValue);
    // if(returnValue != 56.78){
    //     return false;
    // }
    testInput->keyPressed = '9';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-9.0,returnValue);
    // if(returnValue != -9.0){
    //     return false;
    // }
    testInput->keyPressed = '*';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",-9.0,returnValue);
    // if(returnValue != -9.0){
    //     return false;
    // }
    testInput->keyPressed = '*';
    returnValue = keypadInput(testInput);
    printf("expected %f, actual %f\n",0.0,returnValue);
    // if(returnValue != 0.0){
    //     return false;
    // }
    return true;
}
