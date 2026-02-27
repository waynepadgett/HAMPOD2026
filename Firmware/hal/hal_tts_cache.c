#include "hal_tts_cache.h"
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CACHE_DIR_ENV "HAMPOD_TTS_CACHE_DIR"
#define DEFAULT_CACHE_DIR ".cache/hampod/tts"
#define MAX_DISK_CACHE_SIZE (10ULL * 1024ULL * 1024ULL * 1024ULL) /* 10GB */

static char cache_dir_path[512] = {0};
static uint64_t current_cache_size = 0;
static int cache_initialized = 0;

/* DJB2 Hash function */
static uint32_t djb2_hash(const char *str) {
  uint32_t hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return hash;
}

/* Construct file path from hash */
static void get_file_path(uint32_t hash, char *path, size_t path_len) {
  snprintf(path, path_len, "%s/%08x.pcm", cache_dir_path, hash);
}

/* Recursive mkdir */
static int mkdir_p(const char *path) {
  char tmp[512];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", path);
  len = strlen(tmp);
  if (tmp[len - 1] == '/')
    tmp[len - 1] = 0;
  for (p = tmp + 1; *p; p++)
    if (*p == '/') {
      *p = 0;
      if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
      }
      *p = '/';
    }
  if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
    return -1;
  }
  return 0;
}

/* Calculate initial total cache size */
static void compute_cache_size(void) {
  DIR *dir;
  struct dirent *ent;
  struct stat st;
  char filepath[1024];

  current_cache_size = 0;
  if ((dir = opendir(cache_dir_path)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strstr(ent->d_name, ".pcm")) {
        snprintf(filepath, sizeof(filepath), "%s/%s", cache_dir_path,
                 ent->d_name);
        if (stat(filepath, &st) == 0) {
          current_cache_size += st.st_size;
        }
      }
    }
    closedir(dir);
  }
}

int hal_tts_cache_init(void) {
  const char *env_dir = getenv(CACHE_DIR_ENV);
  if (env_dir) {
    snprintf(cache_dir_path, sizeof(cache_dir_path), "%s", env_dir);
  } else {
    const char *home = getenv("HOME");
    if (home) {
      snprintf(cache_dir_path, sizeof(cache_dir_path), "%s/%s", home,
               DEFAULT_CACHE_DIR);
    } else {
      snprintf(cache_dir_path, sizeof(cache_dir_path), "/tmp/%s",
               DEFAULT_CACHE_DIR);
    }
  }

  if (mkdir_p(cache_dir_path) != 0) {
    fprintf(stderr, "HAL TTS CACHE: Failed to create cache directory %s\n",
            cache_dir_path);
    return -1;
  }

  compute_cache_size();
  cache_initialized = 1;
  printf("HAL TTS CACHE: Initialized at %s, current size = %llu bytes\n",
         cache_dir_path, (unsigned long long)current_cache_size);
  return 0;
}

int hal_tts_cache_lookup(const char *text, int16_t **samples,
                         size_t *num_samples) {
  if (!cache_initialized) {
    if (hal_tts_cache_init() != 0)
      return -1;
  }

  uint32_t hash = djb2_hash(text);
  char filepath[512];
  get_file_path(hash, filepath, sizeof(filepath));

  FILE *f = fopen(filepath, "rb");
  if (!f)
    return -1; // Cache miss

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (size <= 0) {
    fclose(f);
    return -1;
  }

  *samples = (int16_t *)malloc(size);
  if (!*samples) {
    fclose(f);
    return -1;
  }

  size_t read_bytes = fread(*samples, 1, size, f);
  fclose(f);

  if (read_bytes != (size_t)size) {
    free(*samples);
    *samples = NULL;
    return -1;
  }

  *num_samples = size / sizeof(int16_t);
  return 0; // Hit
}

void hal_tts_cache_release(int16_t *samples) {
  if (samples) {
    free(samples);
  }
}

int hal_tts_cache_store(const char *text, const int16_t *samples,
                        size_t num_samples) {
  if (!cache_initialized) {
    if (hal_tts_cache_init() != 0)
      return -1;
  }

  size_t size_bytes = num_samples * sizeof(int16_t);

  // MVP phase: enforce hard limit (simplistic)
  if (current_cache_size + size_bytes > MAX_DISK_CACHE_SIZE) {
    fprintf(stderr, "HAL TTS CACHE: Disk cache full\n");
    return -1;
  }

  uint32_t hash = djb2_hash(text);
  char filepath[512];
  get_file_path(hash, filepath, sizeof(filepath));

  FILE *f = fopen(filepath, "wb");
  if (!f)
    return -1;

  size_t written = fwrite(samples, sizeof(int16_t), num_samples, f);
  fclose(f);

  if (written == num_samples) {
    current_cache_size += size_bytes;
    return 0;
  }

  // Cleanup on partial write
  remove(filepath);
  return -1;
}

void hal_tts_cache_cleanup(void) { cache_initialized = 0; }

int hal_tts_cache_clear(void) {
  if (!cache_initialized) {
    if (hal_tts_cache_init() != 0)
      return -1;
  }

  DIR *dir;
  struct dirent *ent;
  char filepath[1024];

  if ((dir = opendir(cache_dir_path)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strstr(ent->d_name, ".pcm")) {
        snprintf(filepath, sizeof(filepath), "%s/%s", cache_dir_path,
                 ent->d_name);
        remove(filepath);
      }
    }
    closedir(dir);
  }
  current_cache_size = 0;
  return 0;
}
