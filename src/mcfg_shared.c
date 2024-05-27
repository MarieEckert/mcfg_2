/* mcfg_shared.c ; marie config format internal shared functions
 * for MCFG/2
 *
 * Copyright (c) 2023, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#include "mcfg_shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *malloc_or_die(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    fprintf(stderr, "malloc_or_die failed. (size = %zu)\n", size);
    abort();
  }

  return ptr;
}

void *realloc_or_die(void *org, size_t size) {
  void *ptr = realloc(org, size);
  if (ptr == NULL) {
    fprintf(stderr, "realloc_or_die failed. (org = %p; size = %zu)\n", org,
            size);
    abort();
  }

  return ptr;
}

bool is_string_empty(char *in) {
  if (in == NULL || in[0] == 0)
    return true;

  size_t len = strlen(in);
  for (size_t i = 0; i < len; i++)
    if (in[i] > ' ')
      return false;

  return true;
}

char *remove_newline(char *in) {
  if (in == NULL || strlen(in) == 0)
    return in;

  if (in[strlen(in) - 1] == '\n')
    in[strlen(in) - 1] = 0;

  return in;
}

bool has_newline(char *in) {
  if (in == NULL || strlen(in) == 0)
    return false;

  for (size_t ix = 0; ix < strlen(in); ix++)
    if (in[ix] == '\n')
      return true;

  return false;
}
// Copy src until delimiter char is hit
char *strcpy_until(char *src, char delimiter) {
  int offs = 0;
  while (offs < strlen(src)) {
    if (src[offs] == delimiter)
      break;
    offs++;
  }

  if (offs == 0)
    return strdup("\0");

  char *res = malloc(offs + 1);
  if (res == NULL) {
    return NULL;
  }

  memcpy(res, src, offs);
  res[offs] = 0;

  return res;
}

// Copy backwards from src until src_org or the delimiter char is hit.
char *bstrcpy_until(char *src, char *src_org, char delimiter) {
  int offs = 0;
  while ((src - offs) > src_org) {
    if ((src - offs)[0] == delimiter)
      break;
    offs++;
  }

  if (offs == 0)
    return strdup("\0");

  char *res = malloc(offs + 1);
  if (res == NULL) {
    return NULL;
  }

  memcpy(res, src - offs + 1, offs);
  res[offs] = 0;

  return res;
}

char *find_prev(char *src, char *src_org, char delimiter) {
  int offs = 0;
  while ((src - offs) > src_org) {
    if ((src - offs)[0] == delimiter)
      return src - offs;
    offs++;
  }

  return src - offs;
}
