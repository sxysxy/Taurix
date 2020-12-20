#ifndef TAURIX_FS_PATCH_H
#define TAURIX_FS_PATCH_H

#include <taurix.h>
#define memset ru_memset
#define memcpy ru_memcpy
#define strcpy ru_strcpy
#define strncmp ru_strncmp
#define strncpy ru_strncpy
#define memcmp ru_memcmp
#define strlen ru_strlen
#define strcat ru_strcat

#endif