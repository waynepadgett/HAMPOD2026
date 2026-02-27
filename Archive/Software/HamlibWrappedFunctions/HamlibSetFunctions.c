#include "HamlibSetFunctions.h"
#include "../Radio.h"
#include "../ModeRouting.h"
char* vfo_array[3] = {"VFOA", "VFOB", "VFOC"};
int non_existing_vfo_feature; 
int temp_index = 0; 
char* mode_array[6] = {"AM", "CW", "USB", "LSB", "RTTY", "FM"};

char* set_frequency(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    freq_t value = *((freq_t*)((void**)input)[2]);
    
    char* output = malloc(100); 
    int retcode = rig_set_freq(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "Frequency now %.3f\n", value);
    } else {
        printf("rig_set_freq: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_mode_custom(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t value = *((vfo_t*)((void**)input)[1]);

    char* output = malloc(100);
    int retcode;

    // Get the current mode
    rmode_t current_mode;
    pbwidth_t current_width; 
    retcode = rig_get_mode(rig, RIG_VFO_CURR, &current_mode, &current_width);
    if (retcode != RIG_OK) {
        printf("Rig cannot get mode\n"); 
    }
    PRINTFLEVEL1("Current mode is %s.\n", rig_strrmode(current_mode));

    // Find the index of the current mode
    int current_index;
    for (current_index = 0; current_index < 6; current_index++) {
        if (strcmp(mode_array[current_index], rig_strrmode(current_mode)) == 0) {
            printf("Found index, %s\n", rig_strrmode(current_mode));
            break; // Found the index
        }
    }

    // Try the next mode
    for (int i = 0; i < 6; i++) {
        int next_index = (current_index + i + 1) % 6;
        PRINTFLEVEL1("Next index, %s\n", mode_array[next_index]);
        retcode = rig_set_mode(rig, RIG_VFO_CURR, rig_parse_mode(mode_array[next_index]), rig_passband_normal(rig, rig_parse_mode(mode_array[next_index])));
        if (retcode == RIG_OK) {
            snprintf(output, 100, "Mode set to %s\n", mode_array[next_index]);
            return output; 
        } else {
            printf("Error setting mode: %s\n", rigerror(retcode));
        }
    }
    snprintf(output, 100, "Unable to set mode\n");
    return output;
}

char* set_mode(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    rmode_t mode_value = *((rmode_t*)((void**)input)[2]);
    pbwidth_t width_value = *((pbwidth_t*)((void**)input)[3]);

    char* output = malloc(100); 
    int retcode = rig_set_mode(rig, vfo, mode_value, width_value); 	
    if (retcode == RIG_OK) {
        snprintf(output, 100, "Mode now %s, Passband now %ld\n", rig_strrmode(mode_value), width_value);
    } else {
        printf("set_current_mode: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_vfo_custom(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t value = *((vfo_t*)((void**)input)[1]);

    char* output = malloc(100);
    int retcode;

    // rig_set_vfo(rig, rig_parse_vfo(vfo_array[1]));
    // snprintf(output, 100, "VFO set to %s\n", vfo_array[1]);

    // Get the current VFO
    vfo_t current_vfo;
    retcode = rig_get_vfo(rig, &current_vfo);
    if (retcode != RIG_OK) {
        printf("Rig cannot get VFO\n"); 
        non_existing_vfo_feature = 1; 
    }
    if (non_existing_vfo_feature) {
        // Feature does not exist on rig, which sucks... 
        // This is the only way I could implement a close working idea. Basically we will have an independent current VFO on the Hampod. 
        // If the user does not change the VFO on the rig, this should work. 
        // If they do though, it won't break, it will just take at most two presses until the Hampod catches up to the current on the rig. 
        // Also note, this was found through the ICOM7300 were it kept returning "Feature not available."
        // Feel free to change!!! 
        // Try next VFO
        for (int i = 0; i < 3; i++) {
            int temp_index = (temp_index + i + 1) % 3;
            PRINTFLEVEL1("Next index, %s\n", vfo_array[temp_index]);
            retcode = rig_set_vfo(rig, rig_parse_vfo(vfo_array[temp_index]));
            if (retcode == RIG_OK) {
                snprintf(output, 100, "VFO set to %s\n", vfo_array[temp_index]);
                return output; 
            } else {
                printf("Error setting VFO: %s\n", rigerror(retcode));
            }
        }
    } else {
        // Feature exists on rig. 
        PRINTFLEVEL1("Current VFO is %s.\n", rig_strvfo(current_vfo));

        // Find the index of the current VFO
        int current_index;
        for (current_index = 0; current_index < 3; current_index++) {
            if (strcmp(vfo_array[current_index], rig_strvfo(current_vfo)) == 0) {
                printf("Found index, %s\n", rig_strvfo(current_vfo));
                break; // Found the index
            }
        }

        // Try next VFO
        for (int i = 0; i < 3; i++) {
            int next_index = (current_index + i + 1) % 3;
            PRINTFLEVEL1("Next index, %s\n", vfo_array[next_index]);
            retcode = rig_set_vfo(rig, rig_parse_vfo(vfo_array[next_index]));
            if (retcode == RIG_OK) {
                snprintf(output, 100, "VFO set to %s\n", vfo_array[next_index]);
                return output; 
            } else {
                printf("Error setting VFO: %s\n", rigerror(retcode));
            }
        }
    }
    
    
    snprintf(output, 100, "Unable to set VFO\n");
    return output;
}

char* set_vfo(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t value = *((vfo_t*)((void**)input)[1]);

    char* output = malloc(100); 
    int retcode = rig_set_vfo(rig, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "VFO set to %s\n", rig_strvfo(value));
    } else {
        printf("rig_set_vfo: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* push_to_talk_converter_for_set (ptt_t* input) {
    if (*input == RIG_PTT_ON) {
        return "on"; 
    } else if (input == RIG_PTT_OFF) {
        return "off"; 
    } else {
        return "-1"; 
    }
}

char* set_ptt(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    ptt_t value = *((ptt_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_ptt(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "PTT set to %s\n", push_to_talk_converter_for_set(&value));
    } else {
        printf("rig_set_ptt: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

// char* get_dcd(RIG* rig, vfo_t vfo) {
//     dcd_t value; 
//     int retcode = rig_get_dcd(rig, vfo, &value)
//     if (retcode == RIG_OK) {
//         char* output = malloc(40); 
//         if (value == RIG_PTT_ON) {
//             snprintf(output, 40, "On\n", value);
//         } else {
//             snprintf(output, 40, "Off\n", value);
//         }
//         return output; 
//     } else {
//         printf("rig_get_dcd: error = %s\n", rigerror(retcode));
//         return "-1"; 
//     }
// }

char* set_rit_offset(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    shortfreq_t value = *((shortfreq_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_rit(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "RIT set to %ld\n", value);
    } else {
        printf("rig_set_rit: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_xit_offset(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    shortfreq_t value = *((shortfreq_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_xit(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "XIT set to %ld\n", value);
    } else {
        printf("rig_set_xit: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_tuning_step(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    shortfreq_t value = *((shortfreq_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_ts(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "Tuning step set to %ld\n", value);
    } else {
        printf("rig_set_ts: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_CTCSS_sub_audible_tone(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    tone_t value = *((tone_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_ctcss_tone(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "CTCSS Tone set to %i\n", value);
    } else {
        printf("rig_set_ctcss_tone: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_current_encoding_digitally_coded_squelch_code(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    tone_t value = *((tone_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_dcs_code(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "DCS code set to %i\n", value);
    } else {
        printf("rig_set_dcs_code: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_current_CTCSS_sub_audible_squelch_tone(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    tone_t value = *((tone_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_ctcss_sql(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "CTCSS SQL set to %i\n", value);
    } else {
        printf("rig_set_ctcss_sql: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}

char* set_current_continuous_tone_controlled_squelch_code(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    tone_t value = *((tone_t*)((void**)input)[2]);

    char* output = malloc(100); 
    int retcode = rig_set_dcs_sql(rig, vfo, value); 
    if (retcode == RIG_OK) {
        snprintf(output, 100, "DCS code set to %i\n", value);
    } else {
        printf("rig_set_dcs_sql: error = %s\n", rigerror(retcode));
        snprintf(output, 100, "-1\n");
    }
    return output; 
}
char* set_level(void* input) {
    printf("I get here into the level function\n"); 
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    setting_t setting_value = (setting_t)(uintptr_t)((void**)input)[2];
    value_t value = *((value_t*)((void**)input)[3]); 

    printf("I finish casting\n"); 
	char* output = malloc(100); 
    rig_set_level (rig, vfo, setting_value, value); 
    snprintf(output, 100, "%s set to %f", rig_strlevel(setting_value), value.f); 
    printf("I finish entirely\n"); 
    return output; 
}

char* set_func(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    setting_t setting_value = (setting_t)(uintptr_t)((void**)input)[2];
    int value = (int)(uintptr_t)((void**)input)[3];

    printf("%i", value); 
	char* output = malloc(100); 
    rig_set_func (rig, vfo, setting_value, value); 	
    snprintf(output, 100, "%s set to %d", rig_strfunc(setting_value), value); 
    return output; 
}
