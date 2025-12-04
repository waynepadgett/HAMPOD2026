// Might move to GeneralFunctions later. 
void replaceSlashWithDash(char *str, size_t size) {
    while (*str && size > 1) {
        if (*str == '/') {
            *str = '-';
        }
        str++;
        size--;
    }
}

void removeTextFiles() {
  DIR *d;
  struct dirent *dir;
  d = opendir("StartupFiles/");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type == DT_REG && strstr(dir->d_name, ".txt")) {
        char fullFilePath[300];
        snprintf(fullFilePath, sizeof(fullFilePath), "StartupFiles/%s", dir->d_name);
        remove(fullFilePath);
      }
    }
    closedir(d);
  }
}

void createTextFileList() {
  DIR *d;
  struct dirent *dir;
  FILE *file = fopen("StartupFiles/Company_List.txt", "w");
  if (!file) {
    fprintf(stderr, "Error opening file for writing\n");
    exit(1);
  }

  d = opendir("StartupFiles/");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type == DT_REG && strstr(dir->d_name, "_Model.txt")) {
        char companyName[300];
        strncpy(companyName, dir->d_name, sizeof(companyName) - 1);
        companyName[sizeof(companyName) - 1] = '\0'; 
        // Remove "_Model.txt"
        size_t len = strlen(companyName);
        if (len > 10 && strcmp(&companyName[len - 10], "_Model.txt") == 0) {
          companyName[len - 10] = '\0';
        }
        fprintf(file, "%s\n", companyName);
      }
    }
    exit(1);
  }
  const char *port = "/dev/pts/3";
  strcpy(rig->state.rigport.pathname, port);

  // Rig info list file
  char modifiedMfgName[300];
  strncpy(modifiedMfgName, caps->mfg_name, sizeof(modifiedMfgName) - 1);
  modifiedMfgName[sizeof(modifiedMfgName) - 1] = '\0'; // Ensure null-terminated
  replaceSlashWithDash(modifiedMfgName, sizeof(modifiedMfgName));

  char filename[300];
  snprintf(filename, sizeof(filename), "StartupFiles/%s_Model.txt", modifiedMfgName);

  FILE *file_model = fopen(filename, "a+");
  if (!file_model) {
    fprintf(stderr, "Error opening file for writing to model %s\n", filename);
    exit(1);
  }


  
  fprintf(file_model, "%s, %s\n", caps->mfg_name, caps->model_name);
  fclose(file_model);

  fflush(stdout);

  // Rig id list file
  snprintf(filename, sizeof(filename), "StartupFiles/%s_Hamlib.txt", modifiedMfgName);

  printf("%s\n", filename); 
  FILE *file_hamlib = fopen(filename, "a+");
  if (!file_hamlib) {
    fprintf(stderr, "Error opening file for writing to id %s\n", filename);
    exit(1);
  }

  fprintf(file_hamlib, "%u\n", caps->rig_model);
  fclose(file_hamlib);

  fflush(stdout);

  rig_cleanup(rig);
  return 1;
}

int createRigLists() {
  RIG rig;
  printf("testing rig timeouts when rig powered off\n");

  /* Turn off backend debugging output */
  rig_set_debug_level(RIG_DEBUG_NONE);
  removeTextFiles();
  rig_load_all_backends();
  rig_list_foreach(callback, &rig);
  createTextFileList(); 
  return 0;
}