# Hamlib API Reference

> Source: [Hamlib 4.3 API Documentation](https://hamlib.sourceforge.net/manuals/4.3/index.html)

## Table of Contents

- [Rig (Transceiver) API](#rig-transceiver-api)
  - [Initialization & Cleanup](#initialization--cleanup)
  - [Frequency Control](#frequency-control)
  - [Mode Control](#mode-control)
  - [VFO Control](#vfo-control)
  - [Level Control](#level-control)
  - [Function Control](#function-control)
  - [PTT & Keying](#ptt--keying)
  - [Information & Status](#information--status)
  - [Split Operation](#split-operation)
  - [Tuning & Passband](#tuning--passband)
  - [Memory & Channel](#memory--channel)
  - [Events & Transceive](#events--transceive)
  - [CTCSS & DCS](#ctcss--dcs)
  - [Power Control](#power-control)
  - [Extension Levels & Functions](#extension-levels--functions)
  - [Configuration](#configuration)
- [Rotator API](#rotator-api)
- [Amplifier API](#amplifier-api)
- [Utility Routines API](#utility-routines-api)

---

## Rig (Transceiver) API

### Initialization & Cleanup

| Function | Description |
|----------|-------------|
| `rig_init(rig_model_t rig_model)` | Allocate a new RIG handle and initialize associated data for the rig model. |
| `rig_open(RIG *rig)` | Open the communication channel to the rig. |
| `rig_close(RIG *rig)` | Close the communication channel to the rig. |
| `rig_cleanup(RIG *rig)` | Release a RIG handle and free associated memory. |
| `rig_reset(RIG *rig, rig_reset_t reset)` | Reset the rig (software reset, VFOA reset, etc.). |
| `rig_get_caps(RIG *rig)` | Query rig capabilities. |
| `rig_get_model_list(void)` | Get list of available rig models. |
| `rig_cookie(RIG *rig, rig_cookie_t cookie, char *cookie_addr)` | Cookie management for concurrent rig access. |

### Frequency Control

| Function | Description |
|----------|-------------|
| `rig_set_freq(RIG *rig, vfo_t vfo, freq_t freq)` | Set the frequency of the rig. |
| `rig_get_freq(RIG *rig, vfo_t vfo, freq_t *freq)` | Get the frequency of the rig. |
| `rig_get_freq_range(RIG *rig, vfo_t vfo, freq_t *low, freq_t *high, split_t *split, int *vfo_list)` | Get the frequency range for the rig. |

### Mode Control

| Function | Description |
|----------|-------------|
| `rig_set_mode(RIG *rig, vfo_t vfo, rmode_t mode, pbwidth_t width)` | Set the operating mode (AM, CW, USB, LSB, FM, etc.). |
| `rig_get_mode(RIG *rig, vfo_t vfo, rmode_t *mode, pbwidth_t *width)` | Get the operating mode. |
| `rig_has_mode(RIG *rig, rmode_t mode, pbwidth_t width)` | Check if rig supports the given mode/width. |
| `rig_passband_normal(RIG *rig, rmode_t mode)` | Get the normal passband width for a mode. |
| `rig_passband_narrow(RIG *rig, rmode_t mode)` | Get the narrow passband width for a mode. |
| `rig_passband_wide(RIG *rig, rmode_t mode)` | Get the wide passband width for a mode. |

### VFO Control

| Function | Description |
|----------|-------------|
| `rig_set_vfo(RIG *rig, vfo_t vfo)` | Set the current VFO. |
| `rig_get_vfo(RIG *rig, vfo_t *vfo)` | Get the current VFO. |

### Level Control

| Function | Description |
|----------|-------------|
| `rig_set_level(RIG *rig, vfo_t vfo, setting_t level, value_t val)` | Set a rig level (AF, RF, SQL, ATT, PREAMP, etc.). |
| `rig_get_level(RIG *rig, vfo_t vfo, setting_t level, value_t *val)` | Get a rig level. |
| `rig_has_get_level(RIG *rig, setting_t level)` | Check which levels can be queried. |
| `rig_has_set_level(RIG *rig, setting_t level)` | Check which levels can be set. |
| `rig_set_level_stat(RIG *rig, vfo_t vfo, setting_t level, int status)` | Set a level with integer status value. |
| `rig_get_level_stat(RIG *rig, vfo_t vfo, setting_t level, int *status)` | Get a level as integer status. |

**Common Levels:**
- `RIG_LEVEL_AF` - AF gain
- `RIG_LEVEL_RF` - RF gain
- `RIG_LEVEL_SQL` - Squelch level
- `RIG_LEVEL_ATT` - Attenuator
- `RIG_LEVEL_PREAMP` - Preamp
- `RIG_LEVEL_AGC` - AGC
- `RIG_LEVEL_AF` - AF gain
- `RIG_LEVEL_MICGAIN` - Microphone gain
- `RIG_LEVEL_KEYSPD` - Key speed
- `RIG_LEVEL_NOTCHF` - Notch filter
- `RIG_LEVEL_COMP` - Compression level
- `RIG_LEVEL_RFPOWER` - RF power output
- `RIG_LEVEL_SWR` - SWR reading
- `RIG_LEVEL_ALC` - ALC level
- `RIG_LEVEL_STRENGTH` - Signal strength (S-meter)
- `RIG_LEVEL_RAWSTR` - Raw signal strength
- `RIG_LEVEL_NR` - Noise reduction level
- `RIG_LEVEL_PBT_IN` - Passband tuning (in)
- `RIG_LEVEL_PBT_OUT` - Passband tuning (out)
- `RIG_LEVEL_CWPITCH` - CW pitch
- `RIG_LEVEL_BALANCE` - Balance (dual VFO)
- `RIG_LEVEL_METER` - Meter selection
- `RIG_LEVEL_VOXDELAY` - VOX delay
- `RIG_LEVEL_VOXGAIN` - VOX gain
- `RIG_LEVEL_ANTIVOX` - Anti-VOX
- `RIG_LEVEL_SLOPE_LOW` - Slope tuning low
- `RIG_LEVEL_SLOPE_HIGH` - Slope tuning high
- `RIG_LEVEL_BKIN_DLYMS` - Break-in delay (ms)
- `RIG_LEVEL_MONITOR_GAIN` - Monitor gain
- `RIG_LEVEL_NB` - Noise blanker level

### Function Control

| Function | Description |
|----------|-------------|
| `rig_set_func(RIG *rig, vfo_t vfo, setting_t func, int status)` | Set a rig function (NB, COMP, VOX, TONE, etc.). |
| `rig_get_func(RIG *rig, vfo_t vfo, setting_t func, int *status)` | Get the status of a rig function. |
| `rig_has_get_func(RIG *rig, setting_t func)` | Check which functions can be queried. |
| `rig_has_set_func(RIG *rig, setting_t func)` | Check which functions can be set. |

**Common Functions:**
- `RIG_FUNC_NB` - Noise blanker
- `RIG_FUNC_COMP` - Speech compressor
- `RIG_FUNC_VOX` - VOX
- `RIG_FUNC_TONE` - CTCSS tone encode
- `RIG_FUNC_TSQL` - CTCSS tone squelch
- `RIG_FUNC_SBKIN` - Semi break-in
- `RIG_FUNC_FBKIN` - Full break-in
- `RIG_FUNC_ANF` - Auto notch filter
- `RIG_FUNC_NR` - Noise reduction
- `RIG_FUNC_MON` - Monitor
- `RIG_FUNC_RF` - RTTY filter
- `RIG_FUNC_ARO` - Auto repeat offset
- `RIG_FUNC_LOCK` - Frequency lock
- `RIG_FUNC_MUTE` - Mute
- `RIG_FUNC_VSC` - Voice squelch control
- `RIG_FUNC_REV` - Reverse mode
- `RIG_FUNC_SQL` - Squelch open
- `RIG_FUNC_ABM` - Automatic band memory
- `RIG_FUNC_BC` - Break-in control
- `RIG_FUNC_RIT` - RIT
- `RIG_FUNC_AFC` - AFC
- `RIG_FUNC_SATMODE` - Satellite mode
- `RIG_FUNC_RESUME` - VFO resume
- `RIG_FUNC_XIT` - XIT

### PTT & Keying

| Function | Description |
|----------|-------------|
| `rig_set_ptt(RIG *rig, vfo_t vfo, ptt_t ptt)` | Set the PTT (Push-To-Talk) state. |
| `rig_get_ptt(RIG *rig, vfo_t vfo, ptt_t *ptt)` | Get the PTT state. |
| `rig_get_dcd(RIG *rig, vfo_t vfo, dcd_t *dcd)` | Get the DCD (Data Carrier Detect) state. |
| `rig_set_ctcss_tone(RIG *rig, vfo_t vfo, unsigned int tone)` | Set CTCSS tone (in 0.01 Hz units). |
| `rig_get_ctcss_tone(RIG *rig, vfo_t vfo, unsigned int *tone)` | Get CTCSS tone. |
| `rig_set_dcs_code(RIG *rig, vfo_t vfo, unsigned int code)` | Set DCS code. |
| `rig_get_dcs_code(RIG *rig, vfo_t vfo, unsigned int *code)` | Get DCS code. |
| `rig_set_ctcss_sql(RIG *rig, vfo_t vfo, unsigned int tone)` | Set CTCSS tone squelch. |
| `rig_get_ctcss_sql(RIG *rig, vfo_t vfo, unsigned int *tone)` | Get CTCSS tone squelch. |
| `rig_set_dcs_sql(RIG *rig, vfo_t vfo, unsigned int code)` | Set DCS code squelch. |
| `rig_get_dcs_sql(RIG *rig, vfo_t vfo, unsigned int *code)` | Get DCS code squelch. |

### Information & Status

| Function | Description |
|----------|-------------|
| `rig_get_info(RIG *rig)` | Query general information from the rig (firmware, model, etc.). |
| `rig_get_status(RIG *rig)` | Query status of the rig. |

### Split Operation

| Function | Description |
|----------|-------------|
| `rig_set_split_vfo(RIG *rig, vfo_t vfo, split_t split, vfo_t tx_vfo)` | Set split mode (TX on different VFO). |
| `rig_get_split_vfo(RIG *rig, vfo_t vfo, split_t *split, vfo_t *tx_vfo)` | Get split mode status. |
| `rig_set_split_freq(RIG *rig, vfo_t vfo, freq_t tx_freq)` | Set split TX frequency. |
| `rig_get_split_freq(RIG *rig, vfo_t vfo, freq_t *tx_freq)` | Get split TX frequency. |
| `rig_set_split_mode(RIG *rig, vfo_t vfo, rmode_t mode, pbwidth_t width)` | Set split TX mode. |
| `rig_get_split_mode(RIG *rig, vfo_t vfo, rmode_t *mode, pbwidth_t *width)` | Get split TX mode. |
| `rig_set_split_freq_mode(RIG *rig, vfo_t vfo, freq_t tx_freq, rmode_t mode, pbwidth_t width)` | Set split TX frequency and mode. |
| `rig_get_split_freq_mode(RIG *rig, vfo_t vfo, freq_t *tx_freq, rmode_t *mode, pbwidth_t *width)` | Get split TX frequency and mode. |
| `rig_set_rit(RIG *rig, vfo_t vfo, shortfreq_t rit)` | Set RIT offset. |
| `rig_get_rit(RIG *rig, vfo_t vfo, shortfreq_t *rit)` | Get RIT offset. |
| `rig_set_xit(RIG *rig, vfo_t vfo, shortfreq_t xit)` | Set XIT offset. |
| `rig_get_xit(RIG *rig, vfo_t vfo, shortfreq_t *xit)` | Get XIT offset. |

### Tuning & Passband

| Function | Description |
|----------|-------------|
| `rig_set_ts(RIG *rig, vfo_t vfo, shortfreq_t ts)` | Set tuning step. |
| `rig_get_ts(RIG *rig, vfo_t vfo, shortfreq_t *ts)` | Get tuning step. |
| `rig_set_passband(RIG *rig, vfo_t vfo, pbwidth_t width)` | Set passband width. |
| `rig_get_passband(RIG *rig, vfo_t vfo, pbwidth_t *width)` | Get passband width. |
| `rig_has_get_passband(RIG *rig, pbwidth_t width)` | Check if passband can be queried. |
| `rig_has_set_passband(RIG *rig, pbwidth_t width)` | Check if passband can be set. |
| `rig_set_notch(RIG *rig, vfo_t vfo, freq_t notch, int reversed)` | Set notch filter frequency. |
| `rig_get_notch(RIG *rig, vfo_t vfo, freq_t *notch, int *reversed)` | Get notch filter frequency. |
| `rig_set_rit_off(RIG *rig, vfo_t vfo)` | Turn RIT off. |
| `rig_get_rit_off(RIG *rig, vfo_t vfo)` | Turn RIT off (get). |

### Memory & Channel

| Function | Description |
|----------|-------------|
| `rig_set_channel(RIG *rig, const channel_t *chan)` | Set a memory channel. |
| `rig_get_channel(RIG *rig, channel_t *chan)` | Get a memory channel. |
| `rig_set_mem(RIG *rig, vfo_t vfo, int ch)` | Set memory channel. |
| `rig_get_mem(RIG *rig, vfo_t vfo, int *ch)` | Get memory channel. |

### Events & Transceive

| Function | Description |
|----------|-------------|
| `rig_set_trn(RIG *rig, int trn)` | Set transceive mode (rig notifies host of events). |
| `rig_get_trn(RIG *rig, int *trn)` | Get transceive mode. |

### CTCSS & DCS

| Function | Description |
|----------|-------------|
| `rig_ctcss_list(RIG *rig)` | Get list of supported CTCSS tones. |
| `rig_dcs_list(RIG *rig)` | Get list of supported DCS codes. |
| `rig_ctcss_tone(RIG *rig, freq_t freq)` | Find nearest CTCSS tone for frequency. |
| `rig_ctcss_freq(RIG *rig, unsigned int tone)` | Find frequency for CTCSS tone code. |

### Power Control

| Function | Description |
|----------|-------------|
| `rig_set_powerstat(RIG *rig, powerstat_t status)` | Set rig power state (on/off/standby). |
| `rig_get_powerstat(RIG *rig, powerstat_t *status)` | Get rig power state. |

### Extension Levels & Functions

| Function | Description |
|----------|-------------|
| `rig_set_ext_level(RIG *rig, vfo_t vfo, token_t token, value_t val)` | Set an extension level. |
| `rig_get_ext_level(RIG *rig, vfo_t vfo, token_t token, value_t *val)` | Get an extension level. |
| `rig_set_ext_func(RIG *rig, vfo_t vfo, token_t token, int status)` | Set an extension function. |
| `rig_get_ext_func(RIG *rig, vfo_t vfo, token_t token, int *status)` | Get an extension function. |
| `rig_ext_level_foreach(RIG *rig, int (*cfunc)(RIG *, const struct confparams *, rig_ptr_t), rig_ptr_t data)` | Iterate over extension levels. |
| `rig_ext_func_foreach(RIG *rig, int (*cfunc)(RIG *, const struct confparams *, rig_ptr_t), rig_ptr_t data)` | Iterate over extension functions. |
| `rig_ext_lookup(RIG *rig, const char *name)` | Lookup extension by name. |
| `rig_ext_lookup_tok(RIG *rig, token_t token)` | Lookup extension by token. |
| `rig_ext_token_lookup(RIG *rig, const char *name)` | Get token ID from name. |

### Configuration

| Function | Description |
|----------|-------------|
| `rig_set_conf(RIG *rig, token_t token, const char *val)` | Set a configuration parameter. |
| `rig_get_conf(RIG *rig, token_t token, char *val)` | Get a configuration parameter. |
| `rig_token_foreach(RIG *rig, int (*cfunc)(const struct confparams *, rig_ptr_t), rig_ptr_t data)` | Iterate over configuration parameters. |
| `rig_confparam_lookup(RIG *rig, const char *name)` | Lookup configuration parameter by name. |
| `rig_token_lookup(RIG *rig, const char *name)` | Get token ID from name. |

### Miscellaneous

| Function | Description |
|----------|-------------|
| `rig_set_twiddle(RIG *rig, vfo_t vfo, int twiddle)` | Set knob twiddle timeout. |
| `rig_get_twiddle(RIG *rig, vfo_t vfo, int *twiddle)` | Get knob twiddle timeout. |
| `rig_set_ant(RIG *rig, vfo_t vfo, ant_t ant)` | Set antenna. |
| `rig_get_ant(RIG *rig, vfo_t vfo, ant_t *ant)` | Get antenna. |
| `rig_set_power2mW(RIG *rig, unsigned int *powerout, float power, freq_t freq, rmode_t mode)` | Convert power value to milliwatts. |
| `rig_get_mW2power(RIG *rig, float *power, unsigned int powermW, freq_t freq, rmode_t mode)` | Convert milliwatts to power value. |
| `rig_send_cmd(RIG *rig, vfo_t vfo, const char *cmd)` | Send raw command to rig. |
| `rig_recv_cmd(RIG *rig, vfo_t vfo, char *data, int datasize, int readlen)` | Receive raw response from rig. |
| `rig_set_targetable_vfo(RIG *rig, vfo_t vfo)` | Set targetable VFO mode. |
| `rig_get_targetable_vfo(RIG *rig, vfo_t *vfo)` | Get targetable VFO mode. |

---

## Rotator API

| Function | Description |
|----------|-------------|
| `rot_init(rot_model_t rot_model)` | Allocate a new ROT handle. |
| `rot_open(ROT *rot)` | Open the communication channel to the rotator. |
| `rot_close(ROT *rot)` | Close the communication channel to the rotator. |
| `rot_cleanup(ROT *rot)` | Release a ROT handle and free associated memory. |
| `rot_set_position(ROT *rot, azimuth_t azimuth, elevation_t elevation)` | Set the azimuth and elevation. |
| `rot_get_position(ROT *rot, azimuth_t *azimuth, elevation_t *elevation)` | Get the azimuth and elevation. |
| `rot_park(ROT *rot)` | Park the rotator. |
| `rot_stop(ROT *rot)` | Stop the rotator. |
| `rot_reset(ROT *rot, rot_reset_t reset)` | Reset the rotator. |
| `rot_move(ROT *rot, int direction, int speed)` | Move rotator in direction at speed. |
| `rot_get_info(ROT *rot)` | Get general information from the rotator. |
| `rot_get_status(ROT *rot, rot_status_t *status)` | Query status flags of the rotator. |
| `rot_set_level(ROT *rot, setting_t level, value_t val)` | Set a rotator level. |
| `rot_get_level(ROT *rot, setting_t level, value_t *val)` | Get a rotator level. |
| `rot_set_func(ROT *rot, setting_t func, int status)` | Set a rotator function. |
| `rot_get_func(ROT *rot, setting_t func, int *status)` | Get a rotator function. |
| `rot_set_conf(ROT *rot, token_t token, const char *val)` | Set a configuration parameter. |
| `rot_get_conf(ROT *rot, token_t token, char *val)` | Get a configuration parameter. |
| `rot_token_foreach(ROT *rot, int (*cfunc)(const struct confparams *, rig_ptr_t), rig_ptr_t data)` | Iterate over configuration parameters. |
| `rot_confparam_lookup(ROT *rot, const char *name)` | Lookup configuration parameter by name. |
| `rot_token_lookup(ROT *rot, const char *name)` | Get token ID from name. |
| `rot_ext_level_foreach(ROT *rot, int (*cfunc)(ROT *, const struct confparams *, rig_ptr_t), rig_ptr_t data)` | Iterate over extension levels. |
| `rot_ext_func_foreach(ROT *rot, int (*cfunc)(ROT *, const struct confparams *, rig_ptr_t), rig_ptr_t data)` | Iterate over extension functions. |
| `rot_ext_parm_foreach(ROT *rot, int (*cfunc)(ROT *, const struct confparams *, rig_ptr_t), rig_ptr_t data)` | Iterate over extension parameters. |
| `rot_ext_lookup(ROT *rot, const char *name)` | Lookup extension by name. |
| `rot_ext_lookup_tok(ROT *rot, token_t token)` | Lookup extension by token. |
| `rot_ext_token_lookup(ROT *rot, const char *name)` | Get token ID from name. |
| `rot_set_ext_level(ROT *rot, token_t token, value_t val)` | Set an extension level. |
| `rot_get_ext_level(ROT *rot, token_t token, value_t *val)` | Get an extension level. |
| `rot_set_ext_func(ROT *rot, token_t token, int status)` | Set an extension function. |
| `rot_get_ext_func(ROT *rot, token_t token, int *status)` | Get an extension function. |
| `rot_set_ext_parm(ROT *rot, token_t token, value_t val)` | Set an extension parameter. |
| `rot_get_ext_parm(ROT *rot, token_t token, value_t *val)` | Get an extension parameter. |
| `rot_has_get_level(ROT *rot, setting_t level)` | Check which levels can be queried. |
| `rot_has_set_level(ROT *rot, setting_t level)` | Check which levels can be set. |
| `rot_has_get_func(ROT *rot, setting_t func)` | Check which functions can be queried. |
| `rot_has_set_func(ROT *rot, setting_t func)` | Check which functions can be set. |
| `rot_has_get_parm(ROT *rot, setting_t parm)` | Check which parameters can be queried. |
| `rot_has_set_parm(ROT *rot, setting_t parm)` | Check which parameters can be set. |

### Rotator Movement Macros

| Macro | Description |
|-------|-------------|
| `ROT_MOVE_UP` | Move up (elevation). |
| `ROT_MOVE_DOWN` | Move down (elevation). |
| `ROT_MOVE_LEFT` | Move left (azimuth CCW). |
| `ROT_MOVE_CCW` | Move counterclockwise. |
| `ROT_MOVE_RIGHT` | Move right (azimuth CW). |
| `ROT_MOVE_CW` | Move clockwise. |
| `ROT_SPEED_NOCHANGE` | Don't change speed. |

---

## Amplifier API

| Function | Description |
|----------|-------------|
| `amp_init(amp_model_t amp_model)` | Allocate a new AMP handle. |
| `amp_open(AMP *amp)` | Open the communication channel to the amplifier. |
| `amp_close(AMP *amp)` | Close the communication channel to the amplifier. |
| `amp_cleanup(AMP *amp)` | Release an AMP handle and free associated memory. |
| `amp_reset(AMP *amp, amp_reset_t reset)` | Reset the amplifier. |
| `amp_get_freq(AMP *amp, freq_t *freq)` | Query the operating frequency. |
| `amp_set_freq(AMP *amp, freq_t freq)` | Set the operating frequency. |
| `amp_get_info(AMP *amp)` | Query general information from the amplifier. |
| `amp_get_level(AMP *amp, setting_t level, value_t *val)` | Query the value of a requested level. |
| `amp_get_ext_level(AMP *amp, token_t level, value_t *val)` | Query the value of a requested extension level. |
| `amp_set_powerstat(AMP *amp, powerstat_t status)` | Turn the amplifier On/Off or toggle Standby/Operate. |
| `amp_get_powerstat(AMP *amp, powerstat_t *status)` | Query the power or standby status. |
| `amp_set_conf(AMP *amp, token_t token, const char *val)` | Set an amplifier configuration parameter. |
| `amp_get_conf(AMP *amp, token_t token, char *val)` | Query the value of a configuration parameter. |
| `amp_has_get_level(AMP *amp, setting_t level)` | Check which level settings can be queried. |
| `amp_confparam_lookup(AMP *amp, const char *name)` | Query configuration parameter token by name. |
| `amp_token_lookup(AMP *amp, const char *name)` | Get token ID from name. |
| `amp_ext_level_foreach(AMP *amp, int (*cfunc)(AMP *, const struct confparams *, amp_ptr_t), amp_ptr_t data)` | Iterate over extension levels. |
| `amp_ext_parm_foreach(AMP *amp, int (*cfunc)(AMP *, const struct confparams *, amp_ptr_t), amp_ptr_t data)` | Iterate over extension parameters. |
| `amp_ext_lookup(AMP *amp, const char *name)` | Lookup extension by name. |
| `amp_ext_lookup_tok(AMP *amp, token_t token)` | Lookup extension by token. |
| `amp_ext_token_lookup(AMP *amp, const char *name)` | Get token ID from name. |

---

## Utility Routines API

### Coordinate Conversion

| Function | Description |
|----------|-------------|
| `dms2dec(int degrees, int minutes, double seconds, int sw)` | Convert DMS notation to decimal degrees (D.DDD). |
| `dmmm2dec(int degrees, double minutes, double seconds, int sw)` | Convert D M.MMM notation to decimal degrees. |
| `dec2dms(double dec, int *degrees, int *minutes, double *seconds, int *sw)` | Convert decimal degrees to DMS notation. |
| `dec2dmmm(double dec, int *degrees, double *minutes, int *sw)` | Convert decimal degrees to D M.MMM notation. |

### Grid Square (Maidenhead) Conversion

| Function | Description |
|----------|-------------|
| `locator2longlat(double *longitude, double *latitude, const char *locator)` | Convert QRA locator to Longitude/Latitude. |
| `longlat2locator(double longitude, double latitude, char *locator, int pair_count)` | Convert longitude/latitude to QRA locator. |

### Distance & Bearing

| Function | Description |
|----------|-------------|
| `qrb(double lon1, double lat1, double lon2, double lat2, double *distance, double *azimuth)` | Calculate distance and bearing between two points. |
| `distance_long_path(double distance)` | Calculate long path distance. |
| `azimuth_long_path(double azimuth)` | Calculate long path bearing. |

---

## Common Constants

### VFO Constants

| Constant | Description |
|----------|-------------|
| `RIG_VFO_NONE` | VFO unknown. |
| `RIG_VFO_A` | VFO A. |
| `RIG_VFO_B` | VFO B. |
| `RIG_VFO_C` | VFO C. |
| `RIG_VFO_SUB` | Sub receiver. |
| `RIG_VFO_MAIN` | Main receiver. |
| `RIG_VFO_CURR` | Current VFO. |
| `RIG_VFO_TX` | Transmit VFO. |
| `RIG_VFO_RX` | Receive VFO. |
| `RIG_VFO_ALL` | All VFOs. |

### Mode Constants

| Constant | Description |
|----------|-------------|
| `RIG_MODE_NONE` | No mode. |
| `RIG_MODE_AM` | AM mode. |
| `RIG_MODE_CW` | CW mode. |
| `RIG_MODE_USB` | USB mode. |
| `RIG_MODE_LSB` | LSB mode. |
| `RIG_MODE_RTTY` | RTTY mode. |
| `RIG_MODE_FM` | FM mode. |
| `RIG_MODE_WFM` | Wide FM mode. |
| `RIG_MODE_CWR` | CW Reverse mode. |
| `RIG_MODE_RTTYR` | RTTY Reverse mode. |
| `RIG_MODE_AMS` | AMS mode. |
| `RIG_MODE_PKTLSB` | Packet LSB mode. |
| `RIG_MODE_PKTUSB` | Packet USB mode. |
| `RIG_MODE_PKTFM` | Packet FM mode. |
| `RIG_MODE_ECSSUSB` | ECSS USB mode. |
| `RIG_MODE_ECSSLSB` | ECSS LSB mode. |
| `RIG_MODE_FAX` | FAX mode. |
| `RIG_MODE_SAM` | Synchronous AM. |
| `RIG_MODE_SAL` | Synchronous AM Lower. |
| `RIG_MODE_SAH` | Synchronous AM Upper. |
| `RIG_MODE_DSB` | DSB mode. |
| `RIG_MODE_FMN` | Narrow FM mode. |
| `RIG_MODE_PKTAM` | Packet AM mode. |
| `RIG_MODE_P25` | P25 digital mode. |
| `RIG_MODE_DSTAR` | D-STAR digital mode. |
| `RIG_MODE_DPMR` | dPMR digital mode. |
| `RIG_MODE_NXDNVN` | NXDN VN mode. |
| `RIG_MODE_NXDN_N` | NXDN N mode. |
| `RIG_MODE_DCR` | DCR mode. |
| `RIG_MODE_AMN` | Narrow AM mode. |
| `RIG_MODE_PSK` | PSK mode. |
| `RIG_MODE_PSKR` | PSK Reverse mode. |
| `RIG_MODE_DD` | Digital Data mode. |
| `RIG_MODE_C4FM` | C4FM mode. |
| `RIG_MODE_PKTFMN` | Packet FM Narrow mode. |
| `RIG_MODE_SPEC` | Spectrum mode. |
| `RIG_MODE_CWN` | Narrow CW mode. |

### Error Codes

| Constant | Description |
|----------|-------------|
| `RIG_OK` | Operation successful. |
| `RIG_EINVAL` | Invalid parameter. |
| `RIG_ENIMPL` | Not implemented. |
| `RIG_ENAVAIL` | Capability not available. |
| `RIG_ENOTFOUND` | Rig not found. |
| `RIG_EBUSY` | Rig busy. |
| `RIG_EIO` | I/O error. |
| `RIG_EINTERNAL` | Internal Hamlib error. |
| `RIG_EINVAL` | Invalid parameter. |
| `RIG_ECONF` | Configuration error. |
| `RIG_ENOMEM` | Memory allocation error. |
| `RIG_ETIMEOUT` | Timeout. |
| `RIG_EBUSY` | Bus busy. |
| `RIG_EINVAL` | Invalid argument. |

### Power State Constants

| Constant | Description |
|----------|-------------|
| `RIG_POWER_OFF` | Power off. |
| `RIG_POWER_ON` | Power on. |
| `RIG_POWER_STANDBY` | Standby mode. |
| `RIG_POWER_OPERATE` | Operate mode. |
| `RIG_POWER_UNKNOWN` | Unknown power state. |

---

## Usage Example (C)

```c
#include <hamlib/rig.h>

int main(void)
{
    RIG *my_rig;
    freq_t freq;
    vfo_t vfo;
    int retcode;

    /* Initialize rig model (1 = MODEL_DUMMY, replace with your rig) */
    my_rig = rig_init(RIG_MODEL_DUMMY);

    if (!my_rig) {
        fprintf(stderr, "Unknown rig model %d\n", RIG_MODEL_DUMMY);
        return 1;
    }

    /* Set serial parameters */
    strncpy(my_rig->state.rigport.pathname, "/dev/ttyUSB0", HAMLIB_MAX_PATHLEN - 1);

    /* Open connection to rig */
    retcode = rig_open(my_rig);
    if (retcode != RIG_OK) {
        fprintf(stderr, "rig_open: %s\n", rigerror(retcode));
        return 1;
    }

    /* Set frequency */
    retcode = rig_set_freq(my_rig, RIG_VFO_A, 14250000);
    if (retcode != RIG_OK) {
        fprintf(stderr, "rig_set_freq: %s\n", rigerror(retcode));
    }

    /* Get frequency */
    retcode = rig_get_freq(my_rig, RIG_VFO_A, &freq);
    if (retcode == RIG_OK) {
        printf("Freq: %" PRIfreq " Hz\n", freq);
    }

    /* Set mode */
    retcode = rig_set_mode(my_rig, RIG_VFO_A, RIG_MODE_USB, RIG_PASSBAND_NORMAL);
    if (retcode != RIG_OK) {
        fprintf(stderr, "rig_set_mode: %s\n", rigerror(retcode));
    }

    /* Get current VFO */
    retcode = rig_get_vfo(my_rig, &vfo);
    if (retcode == RIG_OK) {
        printf("VFO: %d\n", vfo);
    }

    /* Cleanup */
    rig_close(my_rig);
    rig_cleanup(my_rig);

    return 0;
}
```

---

## Links

- [Official Hamlib Documentation](https://github.com/Hamlib/Hamlib/wiki/Documentation)
- [Hamlib API Reference (HTML)](https://hamlib.sourceforge.net/manuals/4.3/index.html)
- [Hamlib GitHub Repository](https://github.com/Hamlib/Hamlib)
- [Hamlib Wiki](https://github.com/Hamlib/Hamlib/wiki)
