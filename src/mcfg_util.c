/* mcfg_util ; marie config format utility implementation
 * for MCFG/2
 *
 * Copyright (c) 2023, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 2

#include "mcfg_util.h"

#include "mcfg.h"
#include "shared.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR_CHECK(c, e)                                                        \
  ({                                                                           \
    if (!(c))                                                                  \
      return e;                                                                \
  })

mcfg_path_t mcfg_parse_path(char *path) {
  mcfg_path_t ret = {.absolute = false,
                     .dynfield_path = false,
                     .sector = NULL,
                     .section = NULL,
                     .field = NULL};

  if (path == NULL) {
    return ret;
  }

  const char *path_seperator = "/";

  ssize_t element_count = 0;
  char **elements;
  bool absolute = path[0] == path_seperator[0];

  /* duplicate path so that we can modify it to our hearts content and avoid
   * crashes when this function is called with a string literal
   */
  path = strdup(path);
  char *strtok_saveptr = NULL;
  char *tok = strtok_r(path, path_seperator, &strtok_saveptr);

  while (tok != NULL) {
    if (element_count == 0) {
      elements = malloc(sizeof(char *));

      if (elements == NULL) {
        free(path);
        return (mcfg_path_t){.sector = NULL, .section = NULL, .field = NULL};
      }
    } else {
      elements = realloc(elements, sizeof(char *) * (element_count + 1));
      if (elements == NULL) {
        free(path);
        return (mcfg_path_t){.sector = NULL, .section = NULL, .field = NULL};
      }
    }

    elements[element_count] = malloc(strlen(tok) + 1);
    if (elements[element_count] == NULL) {
      free(path);
      return (mcfg_path_t){.sector = NULL, .section = NULL, .field = NULL};
    }

    strcpy(elements[element_count], tok);

    element_count++;
    tok = strtok_r(NULL, path_seperator, &strtok_saveptr);
  }

  if (element_count == 0) {
    free(path);
    return ret;
  }

  ret.absolute = absolute;

  if (absolute) {
    ret.sector = elements[0];
    if (element_count > 1) {
      ret.section = elements[1];
    }
    if (element_count > 2) {
      ret.field = elements[2];
    }

    goto exit;
  }

  if (element_count == 1) {
    if (elements[0][0] == '%' && elements[0][strlen(elements[0]) - 1] == '%') {
      ret.dynfield_path = true;

      size_t newsize = strlen(elements[0]) - 1;
      char *new = malloc(newsize);
      memcpy(new, elements[0] + 1, newsize - 1);
      new[newsize - 1] = 0;

      free(elements[0]);
      elements[0] = new;
    }

    ret.field = elements[0];
    goto exit;
  }

  ret.field = elements[element_count - 1];
  if (element_count - 2 > -1) {
    ret.section = elements[element_count - 2];
  }
  if (element_count - 3 > -1) {
    ret.sector = elements[element_count - 3];
  }

exit:
  free(elements);
  free(path);
  return ret;
}

void mcfg_free_path(mcfg_path_t path) {
  if (path.sector != NULL) {
    free(path.sector);
  }

  if (path.section != NULL) {
    free(path.section);
  }

  if (path.field != NULL) {
    free(path.field);
  }
}

char *mcfg_path_to_str(mcfg_path_t path) {
  size_t size = path.absolute ? 2 : 1;
  if (path.sector != NULL) {
    size += strlen(path.sector) + 1;
  }

  if (path.section != NULL) {
    size += strlen(path.section) + 1;
  }

  if (path.field != NULL) {
    size += strlen(path.field);
  }

  char *out = malloc(size);
  if (out == NULL) {
    return NULL;
  }

  size_t offs = 0;

  if (path.absolute) {
    out[0] = '/';
    offs++;
  }

  if (path.sector != NULL) {
    strcpy(out + offs, path.sector);
    offs += strlen(path.sector);
    if (path.section != NULL) {
      out[offs] = '/';
      offs++;
    }
  }

  if (path.section != NULL) {
    strcpy(out + offs, path.section);
    offs += strlen(path.section);
    if (path.field != NULL) {
      out[offs] = '/';
      offs++;
    }
  }

  if (path.field != NULL) {
    strcpy(out + offs, path.field);
  }

  return out;
}

mcfg_field_t *mcfg_get_field_by_path(mcfg_file_t *file, mcfg_path_t path) {
  if (path.dynfield_path) {
    return mcfg_get_dynfield(file, path.field);
  }

  if (!path.absolute) {
    return NULL;
  }

  if (path.sector == NULL || path.section == NULL || path.field == NULL) {
    return NULL;
  }

  mcfg_sector_t *sector = mcfg_get_sector(file, path.sector);
  if (sector == NULL) {
    return NULL;
  }

  mcfg_section_t *section = mcfg_get_section(sector, path.section);
  if (section == NULL) {
    return NULL;
  }

  mcfg_field_t *field = mcfg_get_field(section, path.field);
  return field;
}

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

char *mcfg_format_list(mcfg_list_t list, char *prefix, char *postfix) {
  if (list.field_count == 0 || list.fields == NULL) {
    return strdup("");
  }

  char space[2] = " ";
  size_t base_alloc_size = strlen(prefix) + strlen(postfix) + sizeof(space);

  char *seperator = malloc(base_alloc_size);
  ERR_CHECK(seperator != NULL, NULL);

  strcpy(seperator, postfix);
  strcpy(seperator + strlen(postfix), space);
  strcpy(seperator + strlen(postfix) + strlen(space), prefix);

  size_t cpy_offs = 0;
  char *tmp = mcfg_data_to_string(list.fields[0]);
  char *out = malloc(base_alloc_size + strlen(tmp) + strlen(prefix));
  if (out == NULL) {
    goto exit;
  }

  strcpy(out, prefix);
  cpy_offs += strlen(prefix);
  strcpy(out + cpy_offs, tmp);
  cpy_offs += strlen(tmp);
  free(tmp);

  for (size_t ix = 1; ix < list.field_count; ix++) {
    memcpy(out + cpy_offs, seperator, strlen(seperator));
    cpy_offs += strlen(seperator);
    tmp = mcfg_data_to_string(list.fields[ix]);
    out = realloc(out, strlen(out) + strlen(tmp) + strlen(seperator) + 1);
    if (out == NULL) {
      goto exit;
    }

    strcpy(out + cpy_offs, tmp);
    cpy_offs = strlen(out);
    free(tmp);
  }

  size_t prev_end = strlen(out);
  out = realloc(out, strlen(out) + strlen(postfix) + 1);

  if (out != NULL) {
    strcpy(out + prev_end, postfix);
  }

exit:
  free(seperator);

  return out;
}

char *mcfg_list_as_string(mcfg_list_t list) {
  if (list.field_count == 0 || list.fields == NULL) {
    return strdup("");
  }

  size_t cpy_offs = 0;
  char *tmp = mcfg_data_to_string(list.fields[0]);
  char seperator[3] = ", ";
  char *out = malloc(strlen(tmp) + sizeof(seperator));
  if (out == NULL) {
    goto exit;
  }

  strcpy(out, tmp);
  cpy_offs += strlen(out);

  for (size_t ix = 1; ix < list.field_count; ix++) {
    free(tmp);

    strcpy(out + cpy_offs, seperator);
    cpy_offs += strlen(seperator);
    tmp = mcfg_data_to_string(list.fields[ix]);
    out = realloc(out, strlen(out) + strlen(tmp) + sizeof(seperator) + 1);
    if (out == NULL) {
      goto exit;
    }
    strcpy(out + cpy_offs, tmp);
    cpy_offs = strlen(out);
  }

exit:
  free(tmp);
  return out;
}

mcfg_list_t *mcfg_data_as_list(mcfg_field_t field) {
  if (field.data != NULL && field.type == TYPE_LIST) {
    return (mcfg_list_t *)field.data;
  }
  return NULL;
}

char *mcfg_data_as_string(mcfg_field_t field) {
  if (field.data != NULL && field.type == TYPE_STRING) {
    return (char *)field.data;
  }
  return NULL;
}

int mcfg_data_as_int(mcfg_field_t field) {
  if (mcfg_sizeof(field.type) <= 0 || field.size < 1) {
    return 0;
  }

  return (int)*(int *)field.data;
}

mcfg_boolean_t mcfg_data_as_bool(mcfg_field_t field) {
  if (field.type != TYPE_BOOL) {
    return false;
  }

  return (mcfg_boolean_t) * (mcfg_boolean_t *)field.data;
}

uint8_t mcfg_data_as_u8(mcfg_field_t field) {
  if (field.type != TYPE_U8) {
    return 0;
  }

  return (uint8_t) * (uint8_t *)field.data;
}

int8_t mcfg_data_as_i8(mcfg_field_t field) {
  if (field.type != TYPE_I8) {
    return 0;
  }

  return (int8_t) * (int8_t *)field.data;
}

uint16_t mcfg_data_as_u16(mcfg_field_t field) {
  if (field.type != TYPE_U16) {
    return 0;
  }

  return (uint16_t) * (uint16_t *)field.data;
}

int16_t mcfg_data_as_i16(mcfg_field_t field) {
  if (field.type != TYPE_I16) {
    return 0;
  }

  return (int16_t) * (int16_t *)field.data;
}

uint32_t mcfg_data_as_u32(mcfg_field_t field) {
  if (field.type != TYPE_U32) {
    return 0;
  }

  return (uint32_t) * (uint32_t *)field.data;
}

int32_t mcfg_data_as_i32(mcfg_field_t field) {
  if (field.type != TYPE_I32) {
    return 0;
  }

  return (int32_t) * (int32_t *)field.data;
}
