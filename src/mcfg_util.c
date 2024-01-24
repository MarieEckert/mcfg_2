// mcfg_util ; marie config format utility implementation
// for mcfg version 2
//
// Copyright (c) 2023, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#include "mcfg_util.h"

#include "mcfg.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *malloc_or_die(size_t size);
void *realloc_or_die(void *org, size_t size);

char *mcfg_data_to_string(mcfg_field_t field) {
  int64_t num = 0;
  switch (field.type) {
  case TYPE_LIST:
    return mcfg_list_as_string(*((mcfg_list_t *)field.data));
  case TYPE_STRING:
    return strdup(mcfg_data_as_string(field));
  case TYPE_BOOL:
    return strdup(mcfg_data_as_bool(field) ? "true" : "false");
  case TYPE_U8:
    num = mcfg_data_as_u8(field);
    break;
  case TYPE_I8:
    num = mcfg_data_as_i8(field);
    break;
  case TYPE_U16:
    num = mcfg_data_as_u16(field);
    break;
  case TYPE_I16:
    num = mcfg_data_as_i16(field);
    break;
  case TYPE_U32:
    num = mcfg_data_as_u32(field);
    break;
  case TYPE_I32:
    num = mcfg_data_as_i32(field);
    break;
  default:
    return strdup("(invalid type)");
  }

  // mystic string length calculation for number conversion
  // has a "slight" overhead (:
  char *number_ret = malloc((size_t)floor(log10((double)INT64_MAX)));
  sprintf(number_ret, "%ld", num);
  return number_ret;
}

char *mcfg_list_as_string(mcfg_list_t list) {
  if (list.field_count == 0 || list.fields == NULL)
    return strdup("");

  size_t cpy_offs = 0;
  char *tmp = mcfg_data_to_string(list.fields[0]);
  char seperator[3] = ", ";
  char *out = malloc(strlen(tmp) + sizeof(seperator));

  strcpy(out, tmp);
  cpy_offs += strlen(out);
  free(tmp);

  for (size_t ix = 1; ix < list.field_count; ix++) {
    strcpy(out + cpy_offs, seperator);
    cpy_offs += strlen(seperator);
    tmp = mcfg_data_to_string(list.fields[ix]);
    out = realloc(out, strlen(out) + strlen(tmp) + sizeof(seperator) + 1);
    strcpy(out + cpy_offs, tmp);
    cpy_offs = strlen(out);
    free(tmp);
  }

  return out;
}

char *mcfg_data_as_string(mcfg_field_t field) {
  if (field.data != NULL && field.type == TYPE_STRING)
    return (char *)field.data;
  return NULL;
}

size_t _max(size_t a, size_t b) {
  return a > b ? a : b;
}

// Helper function to resize
void _append_char(char **dest, size_t wix, size_t *dest_size, char chr) {
  if (wix >= *dest_size) {
    size_t size_diff = wix - *dest_size;
    size_t new_size = _max(MCFG_EMBED_FORMAT_RESIZE_AMOUNT, size_diff);

    *dest = realloc_or_die(*dest, new_size);
    *dest_size = new_size;
  }

  *dest[wix] = chr;
}

char *mcfg_format_field_embeds(mcfg_field_t field, mcfg_file_t file) {
  if (field.data == NULL || field.type != TYPE_STRING)
    return NULL;

  const char EMBED_PREFIX = '$';
  const char EMBED_OPENING = '(';
  const char EMBED_CLOSING = ')';

  char *input = mcfg_data_as_string(field);
  size_t input_len = strlen(input);

  char *result = malloc_or_die(input_len + 1);
  size_t current_result_size = input_len + 1;

  size_t wix = 0;
  bool escaping = false;

  bool building_embed_opening = false;
  bool building_field_name = false;
  char *embedded_field;
  size_t embedded_field_name_start = 0;
  size_t embedded_field_name_end = 0;

  for (size_t ix = 0; ix < input_len; ix++) {
    switch (input[ix]) {
    case '\\':
      if (escaping) {
        _append_char(&result, wix, &current_result_size, input[ix]);
        wix++;
      }
      escaping = !escaping;
      continue;
    case EMBED_PREFIX:
      if (!escaping) {
        building_embed_opening = true;
      } else {
        _append_char(&result, wix, &current_result_size, input[ix]);
        wix++;
      }
      continue;
    case EMBED_OPENING:
      if (building_embed_opening)
        building_embed_opening = false;
      if (!escaping) {
        building_field_name = true;
        embedded_field_name_start = ix + 1;
      } else {
        _append_char(&result, wix, &current_result_size, input[ix]);
        wix++;
      }
      continue;
    case EMBED_CLOSING:
      if (building_field_name) {
        building_field_name = false;
        size_t _len = (ix - 1 ) - embedded_field_name_start;
        embedded_field = malloc_or_die(_len);
        memcpy(embedded_field, input + embedded_field_name_start, _len);
        eprintf("MCFG_UTIL DEBUG: FIELD NAME = %s\n");
        free(embedded_field);
        // TODO: Handle embedding
        continue;
      }
    
      _append_char(&result, wix, &current_result_size, input[ix]);
      wix++;
      continue;
    default:
      if (!building_field_name) {
        _append_char(&result, wix, &current_result_size, input[ix]);
        wix++;
      }
      continue;
    }
  }

  return result;
}

int mcfg_data_as_int(mcfg_field_t field) {
  if (mcfg_sizeof(field.type) <= 0 || field.size < 1)
    return 0;

  return (int)*(int *)field.data;
}

mcfg_boolean_t mcfg_data_as_bool(mcfg_field_t field) {
  if (field.type != TYPE_BOOL)
    return false;

  return (mcfg_boolean_t) * (mcfg_boolean_t *)field.data;
}

uint8_t mcfg_data_as_u8(mcfg_field_t field) {
  if (field.type != TYPE_U8)
    return 0;

  return (uint8_t) * (uint8_t *)field.data;
}

int8_t mcfg_data_as_i8(mcfg_field_t field) {
  if (field.type != TYPE_I8)
    return 0;

  return (int8_t) * (int8_t *)field.data;
}

uint16_t mcfg_data_as_u16(mcfg_field_t field) {
  if (field.type != TYPE_U16)
    return 0;

  return (uint16_t) * (uint16_t *)field.data;
}

int16_t mcfg_data_as_i16(mcfg_field_t field) {
  if (field.type != TYPE_I16)
    return 0;

  return (int16_t) * (int16_t *)field.data;
}

uint32_t mcfg_data_as_u32(mcfg_field_t field) {
  if (field.type != TYPE_U32)
    return 0;

  return (uint32_t) * (uint32_t *)field.data;
}

int32_t mcfg_data_as_i32(mcfg_field_t field) {
  if (field.type != TYPE_I32)
    return 0;

  return (int32_t) * (int32_t *)field.data;
}
