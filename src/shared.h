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

#define INTERNAL_PREFIX(name) _mcfg_internal_##name

#define strchrnul INTERNAL_PREFIX(strchrnul)
char *strchrnul(const char *str, int c);

#define is_string_empty INTERNAL_PREFIX(is_string_empty)
bool is_string_empty(char *in);

#define remove_newline INTERNAL_PREFIX(remove_newline)
char *remove_newline(char *in);

#define has_newline INTERNAL_PREFIX(has_newline)
bool has_newline(char *in);

#define strcpy_until INTERNAL_PREFIX(strcpy_until)
char *strcpy_until(char *src, char delimiter);

#define bstrcpy_until INTERNAL_PREFIX(bstrcpy_until)
char *bstrcpy_until(char *src, char *src_org, char delimiter);

#define find_prev INTERNAL_PREFIX(find_prev)
char *find_prev(char *src, char *src_org, char delimiter);

#endif
