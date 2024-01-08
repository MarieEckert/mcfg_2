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
