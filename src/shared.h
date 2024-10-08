/* shared.h ; marie config format internal shared function header
 * for MCFG/2
 *
 * Copyright (c) 2023, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>
#include <sys/types.h>

char *strchrnul(const char *str, int c);

bool is_string_empty(char *in);

char *remove_newline(char *in);

bool has_newline(char *in);

char *strcpy_until(char *src, char delimiter);

char *bstrcpy_until(char *src, char *src_org, char delimiter);

char *find_prev(char *src, char *src_org, char delimiter);

#endif
