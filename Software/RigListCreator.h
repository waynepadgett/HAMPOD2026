#ifndef HAMPOD_SOFT_LIST_CREATE
#define HAMPOD_SOFT_LIST_CREATE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dirent.h>
#include <hamlib/rig.h>
// #include <hamlib/config.h>

#define SERIAL_PORT "/dev/ttyUSB0"

void removeTextFiles();
int callback(const struct rig_caps *caps, void *rigp); 
int createRigLists(); 

#ifndef SHAREDLIB
#include "RigListCreator.c"
#endif

#endif