// mcfg_format.c ; marie config format field-formatting implementation
// for MCFG/2
//
// Copyright (c) 2024, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#include "mcfg_format.h"

#include <stdlib.h>

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
  size_t count;
  _embed_t *embeds;
} _embeds_t;

void _free__embeds(_embeds_t embeds) {
  if (embeds.count == 0 || embeds.embeds == NULL) {
    return;
  }
}

_embeds_t _extract_embeds(char *input) {
  _embeds_t res = {.count = 0, .embeds = NULL};

  return res;
}

mcfg_fmt_err_t _embeds_valid(_embeds_t embeds, mcfg_file_t file,
                             mcfg_path_t rel) {
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
