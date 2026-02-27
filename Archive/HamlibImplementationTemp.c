// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <hamlib/rig.h>
// #include <hamlib/riglist.h>
// #include <limits.h>

// #ifdef HAVE_CONFIG_H
// #  include "config.h"
// #endif

// #include "misc.h"

// #define SERIAL_PORT "/dev/ttyUSB0"


// int main() {
//         RIG *my_rig;
//         freq_t freq;
//         int retcode;

//         printf("Connecting to ICOM-7300...\n");

        // rig_set_debug_level(RIG_DEBUG_WARN);

        // rig_load_all_backends();
//      rig_model_t myrig_model = RIG_MODEL_IC7300;
        // rig_model_t myrig_model = 3073;
        // printf("Rig Model: %d\n", myrig_model);
        // my_rig = rig_init(myrig_model);


        // if (!my_rig) {
        //         fprintf(stderr, "Unknown rig num: %d\n", myrig_model);
        //         fprintf(stderr, "Unable to initialize rig\n");
        //         exit(1);
        // }

        // retcode = rig_set_conf(my_rig, rig_token_lookup(my_rig, "rig_pathname"), SERIAL_PORT);

        // if (retcode != RIG_OK) {
        //         fprintf(stderr, "Error setting serial port path: %s\n", rigerror(retcode));
        //         rig_cleanup(my_rig);
        //         exit(2);
        // }

        // retcode = rig_open(my_rig);

        // if (retcode != RIG_OK) {
        //         fprintf(stderr, "Error opening rig connection: %s\n", rigerror(retcode));
        //         rig_cleanup(my_rig);
        //         exit(2);
        // }

        // retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &freq);

        // if (retcode == RIG_OK) {
        //         printf("Current frequency: %f\n", freq);
        // } else {
        //         fprintf(stderr, "Error getting frequency: %s\n", rigerror(retcode));
        // }

        // retcode = rig_set_freq(my_rig, RIG_VFO_CURR, some_number); 

        // if (retcode == RIG_OK) {
        //         printf("Frequency set successfully: %f\n", freq);
        // } else {
        //         fprintf(stderr, "Error setting frequency: %s\n", rigerror(retcode));
        // }

        // rig_close(my_rig);
        // rig_cleanup(my_rig);

        // return 0;