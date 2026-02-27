bool testReadingFile(){

    printf("loading up the file\n");
    char** data = textFileToArray("UnitTesting/testingOutput.txt");
    int index = 0;
    char* expected = "1";
    printf("index %d, Expected %s, Actual %s\n", index,expected, data[index]);
    if(strcmp(expected,data[index]) != 0){
        return false;
    }
    index++;

    expected = "two";
    printf("index %d, Expected %s, Actual %s\n", index,expected, data[index]);
    if(strcmp(expected,data[index]) != 0){
        return false;
    }
    index++;

    expected = "Three";
    printf("index %d, Expected %s, Actual %s\n", index,expected, data[index]);
    if(strcmp(expected,data[index]) != 0){
        return false;
    }
    index++;

    expected = "Four with stuff in it";
    printf("index %d, Expected %s, Actual %s\n", index,expected, data[index]);
    if(strcmp(expected,data[index]) != 0){
        return false;
    }
    index++;

    expected = "The end";
    printf("index %d, Expected %s, Actual %s\n", index,expected, data[index]);
    if(strcmp(expected,data[index]) != 0){
        return false;
    }
    index++;

    expected = "END OF ARRAY";
    printf("index %d, Expected %s, Actual %s\n", index,expected, data[index]);
    if(strcmp(expected,data[index]) != 0){
        return false;
    }
    index++;

    printf("Testing freeing the array\n");
    freeFileArray(data);
    return true;
}