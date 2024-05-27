// mcfg_format.c ; marie config format field-formatting implementation
// for MCFG/2
//
// Copyright (c) 2024, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#include "mcfg_format.h"

#include "mcfg_shared.h"

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

/**
 * @brief This macro is inteded for use inside _format, to ensure that we never
 * write outside of the buffer.
 * @note Causes to return with error on (re)alloction failure!
 *
 * @param b The buffer
 * @param a The allocated buffer size
 * @param s The would-be size after writing to it
 */
#define APPEND_CHECK(b, a, s)                                                  \
  if (s > a) {                                                                 \
    if (b == NULL) {                                                           \
      b = FMTMALLOC(s);                                                        \
    } else {                                                                   \
      b = FMTREALLOC(b, s);                                                    \
    }                                                                          \
  }

typedef struct _embed {
  /** @brief the position at which the embed starts in the input */
  size_t pos;

  /**
   * @brief the position where the embed ends in the input (position of the
   * closing bracket + 1)
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

// Helper function for path relativity
mcfg_path_t _insert_path_elems(mcfg_path_t src, mcfg_path_t rel) {
  if (src.sector == NULL) {
    src.sector = rel.sector != NULL ? strdup(rel.sector) : strdup("(null)");
    src.absolute = true;
  }

  if (src.section == NULL)
    src.section = rel.section != NULL ? strdup(rel.section) : strdup("(null)");

  if (src.field == NULL)
    src.field = rel.field != NULL ? strdup(rel.field) : strdup("(null)");

  return src;
}

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

/**
 * @brief Extracts field embeds from the given input
 */
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
      field_pos = ix - 1;
      break;
    case ')':
      if (!building_field_name) {
        break;
      }

      building_field_name = false;
      field_name[field_name_wix] = 0;
      field_src_end_pos = ix + 1;
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

/* TODO: Remove once debugging is done. "embed validity" does not exist */
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
      .formatted_size = strlen(input) + 1,
      .formatted = NULL,
  };

  res.formatted = FMTMALLOC(res.formatted_size);
  if (embeds.count == 0) {
    strcpy(res.formatted, input);
    return res;
  }

  size_t cpy_offs = 0;
  size_t write_offs = 0;

  const char _FIELD_NULLPTR[] = "(nullptr)";

  for (size_t ix = 0; ix < embeds.count; ix++) {
    if (write_offs > res.formatted_size) {
      res.formatted_size += 1;
      res.formatted = FMTREALLOC(res.formatted, res.formatted_size);
    }

    _embed_t embed = embeds.embeds[ix];

    /* copy characters from input up until embed */
    memcpy(res.formatted + write_offs, input + cpy_offs, embed.pos - cpy_offs);
    write_offs += embed.pos - cpy_offs;
    cpy_offs = embed.src_end_pos;

    /* get, format & insert field */
    mcfg_path_t path = _insert_path_elems(mcfg_parse_path(embed.field), rel);
    mcfg_field_t *field = mcfg_get_field_by_path(&file, path);

    /* field does not exist, insert (nullptr) as placeholder */
    if (field == NULL) {
      memcpy(res.formatted + write_offs, _FIELD_NULLPTR,
             sizeof(_FIELD_NULLPTR));
      write_offs += sizeof(_FIELD_NULLPTR) - 1;
      continue;
    }

    /* field is a number type */
    if (field->type != TYPE_STRING && field->type != TYPE_LIST) {
      char *str = mcfg_data_to_string(*field);
      const size_t str_len = strlen(str);

      APPEND_CHECK(res.formatted, res.formatted_size, write_offs + str_len);

      memcpy(res.formatted + write_offs, str, str_len);
      write_offs += str_len;
      free(str);
      continue;
    }

    mcfg_fmt_res_t subformat_res;

    /* "sub-"format the field before appending */
    if (field->type == TYPE_LIST) {
      /* TODO: Extract pre- and postfix */

      char *prefix =
          remove_newline(bstrcpy_until(input + embed.pos - 1, input, ' '));
      char *postfix =
          remove_newline(strcpy_until(input + embed.src_end_pos, ' '));

      ERR_CHECK(prefix != NULL, MCFG_FMT_NULLPTR);
      ERR_CHECK(postfix != NULL, MCFG_FMT_NULLPTR);

      /* Move write_offs back to position of last space + 1
       * and set cpy_offs to position of next space
       */
      cpy_offs = (size_t)(strchr(input + embed.src_end_pos, ' ') - input);
      write_offs -= strlen(prefix);

      char *list =
          mcfg_format_list(*mcfg_data_as_list(*field), prefix, postfix);
      ERR_CHECK(list != NULL, MCFG_FMT_NULLPTR);

      subformat_res = mcfg_format_field_embeds_str(list, file, rel);

      free(list);
    } else if (field->type == TYPE_STRING) {
      subformat_res = mcfg_format_field_embeds(*field, file, rel);
    }

    ERR_CHECK(subformat_res.err == MCFG_FMT_OK, subformat_res.err);

    /* append "sub-"formatted field */
    const size_t subformat_len = strlen(subformat_res.formatted);
    APPEND_CHECK(res.formatted, res.formatted_size, write_offs + subformat_len);
    memcpy(res.formatted + write_offs, subformat_res.formatted, subformat_len);
    write_offs += subformat_len;
  }

  if (cpy_offs < strlen(input)) {
    const size_t remaining = strlen(input) - cpy_offs;
    APPEND_CHECK(res.formatted, res.formatted_size, write_offs + remaining);
    memcpy(res.formatted + write_offs, input + cpy_offs, remaining);
    write_offs += remaining;
  }

  if (res.formatted[write_offs] != 0) {
    res.formatted[write_offs] = 0;
    write_offs++;
  }

  /* trim buffer size */
  FMTREALLOC(res.formatted, write_offs);
  res.formatted_size = write_offs;

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

  res = _format(strdup(input), embeds, file, relativity);

exit:
  _free__embeds(embeds);
  return res;
}
