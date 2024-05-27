// mcfg_format.c ; marie config format field-formatting implementation
// for MCFG/2
//
// Copyright (c) 2024, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#include "mcfg_format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This macro checks if the condition (c) is false. If c is false, it is
 * considered an error and it will construct a mcfg_fmt_res_t structure with
 * the err field set to the error (e) and cause the function it was called
 * inside of to return.
 * NOTE: This macro is only applicable in functions with a return type of
 * mcfg_fmt_res_t
 */
#define ERR_CHECK(c, e)                                                        \
  ({                                                                           \
    if (!(c))                                                                  \
      return (mcfg_fmt_res_t){                                                 \
          .err = e, .formatted_size = 0, .formatted = NULL};                   \
  })

#define FMTMALLOC(s)                                                           \
  ({                                                                           \
    void *ret;                                                                 \
    ret = malloc(s);                                                           \
    ERR_CHECK(ret != NULL, MCFG_FMT_MALLOC_FAIL);                              \
    ret;                                                                       \
  })

#define FMTREALLOC(o, s)                                                       \
  ({                                                                           \
    void *ret;                                                                 \
    ret = realloc(o, s);                                                       \
    ERR_CHECK(ret != NULL, MCFG_FMT_MALLOC_FAIL);                              \
    ret;                                                                       \
  })

typedef struct _embed {
  /** @brief the position at which the embed starts in the input */
  size_t pos;

  /**
   * @brief the position where the embed ends in the input (position of the
   * closing bracket)
   */
  size_t src_end_pos;

  /** @brief The given path to the field */
  char *field;
} _embed_t;

typedef struct _embeds {
  mcfg_fmt_err_t err;

  size_t count;
  _embed_t *embeds;
} _embeds_t;

void _free__embeds(_embeds_t embeds) {
  if (embeds.count == 0 || embeds.embeds == NULL) {
    return;
  }
}

mcfg_fmt_err_t _append_embed(_embeds_t *embeds, char *field, size_t pos,
                             size_t src_end_pos) {
  if (embeds == NULL) {
    return MCFG_FMT_NULLPTR;
  }

  const size_t wix = embeds->count;
  embeds->count++;

  if (wix == 0) {
    embeds->embeds = malloc(sizeof(*embeds->embeds));
  } else {
    embeds->embeds =
        realloc(embeds->embeds, sizeof(*embeds->embeds) * embeds->count);
  }

  if (embeds->embeds == NULL) {
    return MCFG_FMT_MALLOC_FAIL;
  }

  embeds->embeds[wix].field = field;
  embeds->embeds[wix].pos = pos;
  embeds->embeds[wix].src_end_pos = src_end_pos;

  return MCFG_FMT_OK;
}

_embeds_t _extract_embeds(char *input) {
  _embeds_t res = {.count = 0, .embeds = NULL};

  const size_t input_len = strlen(input);

  size_t field_pos = 0;         /* see _embed_t.pos */
  size_t field_src_end_pos = 0; /* see _embed_t.src_end_pos */
  size_t field_name_wix = 0;
  char *field_name = malloc(input_len + 1); /* this is a bit much, but it allows
                                             * us avoid reallocs */
  if (field_name == NULL) {
    return res;
  }

  bool escaping = false;
  bool building_embed = false;
  bool building_field_name = false;

  for (size_t ix = 0; ix < input_len; ix++) {
    if (escaping) {
      escaping = false;
      continue;
    }

    switch (input[ix]) {
    case '\\':
      escaping = true;
      break;
    case '$':
      building_embed = true;
      break;
    case '(':
      if (!building_embed) {
        break;
      }

      building_embed = false;
      building_field_name = true;
      field_pos = ix + 1;
      break;
    case ')':
      if (!building_field_name) {
        break;
      }

      building_field_name = false;
      field_name[field_name_wix] = 0;
      field_src_end_pos = ix;
      field_name_wix = 0;

      res.err =
          _append_embed(&res, strdup(field_name), field_pos, field_src_end_pos);
      if (res.err != MCFG_FMT_OK) {
        goto exit;
      }
      break;
    default:
      if (!building_field_name) {
        break;
      }

      field_name[field_name_wix] = input[ix];
      field_name_wix++;
      break;
    }
  }

exit:
  free(field_name);
  return res;
}

mcfg_fmt_err_t _embeds_valid(_embeds_t embeds, mcfg_file_t file,
                             mcfg_path_t rel) {
  for (size_t ix = 0; ix < embeds.count; ix++) {
    _embed_t embed = embeds.embeds[ix];
    printf("%zu: pos = %zu, src_end_pos = %zu, field = %s\n", ix, embed.pos,
           embed.src_end_pos, embed.field);
  }

  return MCFG_FMT_OK;
}

mcfg_fmt_res_t _format(char *input, _embeds_t embeds, mcfg_file_t file,
                       mcfg_path_t rel) {
  mcfg_fmt_res_t res = {
      .err = MCFG_FMT_OK,
      .formatted_size = 0,
      .formatted = NULL,
  };

  return res;
}

/* mcfg_format.h functions */

mcfg_fmt_res_t mcfg_format_field_embeds(mcfg_field_t field, mcfg_file_t file,
                                        mcfg_path_t relativity) {
  ERR_CHECK(field.data != NULL, MCFG_FMT_NULLPTR);
  ERR_CHECK(field.type == TYPE_STRING, MCFG_FMT_INVALID_TYPE);

  char *input = mcfg_data_as_string(field);
  return mcfg_format_field_embeds_str(input, file, relativity);
}

mcfg_fmt_res_t mcfg_format_field_embeds_str(char *input, mcfg_file_t file,
                                            mcfg_path_t relativity) {
  ERR_CHECK(input != NULL, MCFG_FMT_NULLPTR);
  mcfg_fmt_res_t res;

  /* Extract all embeds */
  _embeds_t embeds = _extract_embeds(input);
  if (embeds.err != MCFG_FMT_OK) {
    res.err = embeds.err;
    goto exit;
  }

  /* Ensure all embedded fields exist */
  mcfg_fmt_err_t err = _embeds_valid(embeds, file, relativity);
  if (err != MCFG_FMT_OK) {
    res.err = err;
    goto exit;
  }

  res = _format(input, embeds, file, relativity);

exit:
  _free__embeds(embeds);
  return res;
}
