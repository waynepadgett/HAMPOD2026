#include "HashMap.h"
HashMap* createHashMap( int (*hashFunc)(void*), bool (*comparFunc)(void*,void*)){
    HashMap* map = malloc(sizeof(HashMap));
    map->list = calloc(8,sizeof(void*));
    map->listOfKeys = calloc(8,sizeof(void*));
    map->quantity = 0;
    map->size = 8;
    map->comparFunc = comparFunc;
    map->hashFunc = hashFunc;
    return map;
}
void insertHashMap(HashMap* hashmap,void* data,void* key){
    // PRINTFLEVEL2("SOFTWARE: Hash Start of the inesert funciton\n");
    int hashOfKey = hashmap->hashFunc(key);
    // PRINTFLEVEL2("SOFTWARE: Hash ran the key throug the hashing function\n");
    int index = hashOfKey % hashmap->size;
    PRINTFLEVEL2("SOFTWARE: Hash got the index after the hash of %i\n", index);
    int offset = 0;
    bool placingObject = true;
    // PRINTFLEVEL2("SOFTWARE: Hash going into the while loop\n");
    while(placingObject){
        if(hashmap->list[(index+offset)%hashmap->size] == 0){
            // PRINTFLEVEL2("SOFTWARE: Hash Found an index %i to place item at\n", (index+offset)%hashmap->size);
            placingObject = false;
            hashmap->list[(index+offset)%hashmap->size] = data;
            hashmap->listOfKeys[(index+offset)%hashmap->size] = key;
            hashmap->quantity ++;
        }else if(offset != 0 && (index+offset)%hashmap->size == index){
            //TODO 
            //Grow the list size
            //  PRINTFLEVEL2("SOFTWARE: Hash Growing the size of the hashmap\n");
            growHashMap(hashmap);
            insertHashMap(hashmap,data, key);
            placingObject = false;
        }else{
            //  PRINTFLEVEL2("SOFTWARE: Hash Finding a new offset for the hash\n");
            if(offset == 0){
                offset = 1;
            }else if(offset == 1){
                offset = 2;
            }else{
                offset = offset * offset;
            }
        }
    }//end while
}
void* getHashMap(HashMap* hashmap,void* key){
    int index = hashmap->hashFunc(key)%hashmap->size;
    PRINTFLEVEL2("SOFTWARE: Hash got the index after the hash of %i\n", index);
    int offset = 0;
    bool placingObject = true;
    while(placingObject){
        // PRINTFLEVEL2("SOFTWARE: in while with offset of %i\n", offset);
        int offsetIndex = (index+offset)%hashmap->size;
        void* foundValue = hashmap->listOfKeys[offsetIndex];
        // PRINTFLEVEL2("SOFTWARE: got the value of the offset index of %i to be compared\n",offsetIndex);
        if(foundValue != 0 && hashmap->comparFunc(foundValue, key)){
            // PRINTFLEVEL2("SOFTWARE: Found key in hashmap at index %i\n",index+offset);
            return  hashmap->list[(index+offset)%hashmap->size];
        }else if(offset != 0 && (index+offset)%hashmap->size == index){
            //  PRINTFLEVEL2("SOFTWARE: Could not find key in the hashmap\n");
            return NULL; //failed to find anything
        }
        else{
            // PRINTFLEVEL2("SOFTWARE: Increaseing get offset\n");
            if(offset == 0){
                offset = 1;
            }else if(offset == 1){
                offset = 2;
            }else{
                offset = offset * offset;
            }
        }
    }
    return NULL;
}

bool containsHashMap(HashMap* hashmap, void* key){
    if(getHashMap(hashmap,key) == NULL){
        return false;
    }else{
        return true;
    }
}


void* removeHashMap(HashMap* hashmap,void* key){
    int index = hashmap->hashFunc(key)%hashmap->size;
    int offset = 0;
    bool placingObject = true;
    while(placingObject){
        int offsetIndex = (index+offset)%hashmap->size;
        void* foundValue = hashmap->listOfKeys[offsetIndex];
        if(foundValue != 0 && hashmap->comparFunc(foundValue, key)){
            void* value = hashmap->list[(index+offset)%hashmap->size];
            hashmap->list[(index+offset)%hashmap->size] = 0;
            hashmap->listOfKeys[(index+offset)%hashmap->size] = 0;
            hashmap->quantity --;
            return value;
        }else if(offset != 0 && (index+offset)%hashmap->size == index){
            return NULL; //failed to find anything
        }
        else{
            if(offset == 0){
                offset = 1;
            }else if(offset == 1){
                offset = 2;
            }else{
                offset = offset * offset;
            }
        }
    }
    return NULL;
}
void destroyHashMap(HashMap* hashmap,void (*dataFree)(void*), void(*keyFree)(void*)){
    for(int i = 0; i<hashmap->size;i++){
        if(hashmap->list[i] != 0){
            dataFree(hashmap->list[i]);
        }
        if(hashmap->listOfKeys[i] != 0){
            keyFree(hashmap->listOfKeys[i]);
        }
    }

    free(hashmap->list);
    free(hashmap->listOfKeys);
    free(hashmap);
}
void growHashMap(HashMap* hashmap){
    void** oldList = hashmap->list;
    void** oldKeyList = hashmap->listOfKeys;
    int oldQuantity = hashmap->quantity;
    hashmap->size = hashmap->size*2;
    hashmap->list = calloc(hashmap->size,sizeof(void*));
    hashmap->listOfKeys = calloc(hashmap->size,sizeof(void*));
    for(int i = 0; i<hashmap->size/2;i++){
        if(oldKeyList[i] != 0){
            insertHashMap(hashmap, oldList[i], oldKeyList[i]);
        }
    }
    free(oldList);
    free(oldKeyList);
    hashmap->quantity = oldQuantity;
}

/**
 * This will return a list of all of the entires within a hashmap
 * the order will be consisant but adding new items to the map will NOT add it to the end of this list
*/
void** getAllEntriesHashMap(HashMap* hashmap){
    void** entries = malloc(sizeof(void*) * hashmap->quantity);
    int j = 0;
    for(int i = 0; i<hashmap->size;i++){
        if(hashmap->list[i] != 0){
            entries[j] = hashmap->list[i];
            j++;
        }
    }
    return entries;
}



int StringHash(void* key){
    char* st = (char*) key;
    int hash = 0;
    PRINTFLEVEL2("Creating a hash for the string %s\n", st);
    for(int i = 0; i<strlen(st); i++){
        hash += st[i];
    }
    return hash;
}
bool StringHashCompare(void* a, void* b){
    if(strcmp((char*) a, (char*) b) == 0){
        return true;
    }else{
        return false;
    }
}

void StringHashFree(void* s){
    free((char*) s);
}

void NullHashFree(void* s){
    return;
}
int IntHash(void* key){
    int* data = (int*) key;
    return data[0];
}
bool IntHashCompare(void* a, void* b){
    int* a1 = (int*) a;
    int* b1 = (int*) b;
    // printf("comparing values %i and %i\n",a1[0],b1[0]);
    return a1[0] == b1[0];
}
void IntHashFree(void* i){
    free((int*) i);
}
