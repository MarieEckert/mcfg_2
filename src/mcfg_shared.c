#include "mcfg_shared.h"

#include <stdlib.h>
#include <string.h>

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
