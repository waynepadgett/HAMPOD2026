#include "HamlibGetFunctions.h"
char* get_current_transceive_mode(void* input) {
    RIG* rig = ((void**)input)[0];

    int value; 
    char* output = malloc(40); 
    int retcode = rig_get_trn(rig, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "%i\n", value);
    } else {
        printf("rig_get_trn: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_frequency(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    
    freq_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_freq(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "Frequency now %.3f\n", value);
        printf("I finish getting frequency\n"); 
    } else {
        printf("rig_get_freq: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_mode(void* input) {
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    rmode_t mode_value; 
    pbwidth_t width_value; 
    char* output = malloc(40); 
    int retcode = rig_get_mode(rig, vfo, &mode_value, &width_value); 	
    if (retcode == RIG_OK) {
        snprintf(output, 40, "Mode now %s\n", rig_strrmode(mode_value));
    } else {
        printf("get_current_mode: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_mode_width(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    rmode_t mode_value; 
    pbwidth_t width_value; 
    char* output = malloc(40); 
    int retcode = rig_get_mode(rig, vfo, &mode_value, &width_value); 	
    if (retcode == RIG_OK) {
        
        snprintf(output, 40, "Mode width now %ld\n", width_value);
    } else {
        printf("get_current_mode: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_vfo(void* input) {
    // RIG* rig = (RIG*) input[0]; 

    RIG* rig = ((void**)input)[0];

    vfo_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_vfo(rig, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "VFO now %s\n", rig_strvfo(value));
    } else {
        printf("rig_get_vfo: error = %s\n", rigerror(retcode));
        // Trying new way
        printf("%s\n", rig_strvfo(RIG_VFO_CURR)); 
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* push_to_talk_converter_for_get (ptt_t* input) {
    if (*input == RIG_PTT_ON) {
        return "on"; 
    } else if (*input == RIG_PTT_OFF) {
        return "off"; 
    } else {
        return "-1"; 
    }
}

char* get_ptt(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    ptt_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_ptt(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "PTT now %s\n", push_to_talk_converter_for_get(&value));
    } else {
        printf("rig_get_ptt: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
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

// char* get_current_repeater_shift (RIG* rig, vfo_t vfo) {
//     rptr_shift_t value; 
//     int retcode = rig_get_rptr_shift(rig, vfo, &value); 
//     if (retcode == RIG_OK) {
//         char* output = malloc(40); 
//         snprintf(output, 40, "%s\n", repeater_shift_converter(value));
//         return output; 
//     } else {
//         printf("rig_get_rptr_shift: error = %s\n", rigerror(retcode));
//         return "-1"; 
//     }
// }

// char* repeater_shift_converter (rptr_shift_t* input) {
//     switch (input) {
//         case RIG_RPT_SHIFT_NONE: 
//             return "0"; 
//         case RIG_RPT_SHIFT_MINUS: 
//             return "-"; 
//         case RIG_RPT_SHIFT_PLUS: 
//             return "+"; 
//         default: 
//             return "-1";

//     }
// }

// char* get_current_repeater_offset (RIG* rig, vfo_t vfo) {
//     shortfreq_t value; 
//     int retcode = rig_get_rptr_offs(rig, vfo, &value); 
//     if (retcode == RIG_OK) {
//         char* output = malloc(40); 
//         snprintf(output, 40, "%f\n", repeater_shift_converter(value));
//         return output; 
//     } else {
//         printf("rig_get_rptr_offs: error = %s\n", rigerror(retcode));
//         return "-1"; 
//     }
// }


char* get_current_rit_offset(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    shortfreq_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_rit(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "RIT now %ld\n", value);
    } else {
        printf("rig_get_rit: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_xit_offset(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    shortfreq_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_xit(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "XIT now %ld\n", value);
    } else {
        printf("rig_get_xit: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_tuning_step(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    shortfreq_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_ts(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "Tuning Step now %ld\n", value);
    } else {
        printf("rig_get_ts: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_CTCSS_sub_audible_tone(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    tone_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_ctcss_tone(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "CTCSS tone now %i\n", value);
    } else {
        printf("rig_get_ctcss_tone: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_encoding_digitally_coded_squelch_code(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    tone_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_dcs_code(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "DCS code now %i\n", value);
    } else {
        printf("rig_get_dcs_code: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_CTCSS_sub_audible_squelch_tone(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    tone_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_ctcss_sql(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "CTCSS SQL now %i\n", value);
    } else {
        printf("rig_get_ctcss_sql: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

char* get_current_continuous_tone_controlled_squelch_code(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);

    tone_t value; 
    char* output = malloc(40); 
    int retcode = rig_get_dcs_sql(rig, vfo, &value); 
    if (retcode == RIG_OK) {
        snprintf(output, 40, "DCS SQL now %i\n", value);
    } else {
        printf("rig_get_dcs_sql: error = %s\n", rigerror(retcode));
        snprintf(output, 40, "-1\n");
    }
    return output; 
}

// This is for both the LEVEL and FUNC features. Pretty genius if you ask me. :)
// Hamlib is very thoughtful for giving a common interest between rig_get_level and rig_get_func. 
// This would still work, however, I decided to separate the level and func retrieval systems since it was just creating a mess. 

// char* get_level_or_func_wrapper(void* input) {
//     RIG* rig = (RIG*) input[0]; 
//     vfo_t vfo = *(vfo_t*) input[1]; 
//     char* setting_string = rig_strfunc((setting_t*) input[2]); 

// 	char* output = malloc(40); 
// 	if (strstr("LEVEL", setting_string)) {
//         value_t* value; 
//         rig_get_level (rig, vfo, rig_parse_func(setting_string), &value); 
//         snprintf(output, 40, "%d", value); 
//         return output; 
// 	} else if (strstr("FUNC", rig_strfunc(setting_string))) {
//         int status; 
//         rig_get_func (rig, vfo, rig_parse_func(setting_string), &status); 	
//         snprintf(output, 40, "%d", status); 
//         return output; 
// 	} else {
//         return "-1"; 
//     }
// }

char* get_level(void* input) {
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 
    // setting_t setting_value = *(setting_t*) input[2]; 

    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    setting_t setting_value = (setting_t)(uintptr_t)((void**)input)[2];
    
    char* output = malloc(40); 
    value_t value; 
    rig_get_level (rig, vfo, setting_value, &value); 
    snprintf(output, 40, "%s now %d", rig_strlevel(setting_value), value); 
    return output; 
}

char* get_func(void* input) {
    
    // RIG* rig = (RIG*) input[0]; 
    // vfo_t vfo = *(vfo_t*) input[1]; 
    // setting_t setting_value = *(setting_t*) input[2]; 
    
    RIG* rig = ((void**)input)[0];
    vfo_t vfo = *((vfo_t*)((void**)input)[1]);
    setting_t setting_value = (setting_t)(uintptr_t)((void**)input)[2];
    
    char* output = malloc(40); 
    int status; 
    rig_get_func (rig, vfo, setting_value, &status); 	
    snprintf(output, 40, "%s now %d", rig_strfunc(setting_value), status); 
    return output; 
}
