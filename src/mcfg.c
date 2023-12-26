// mcfg.c ; marie config format parser implementation
// implementation for mcfg version 2
//
// Copyright (c) 2023, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#include "mcfg.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *mcfg_err_string(mcfg_err_t err) {
  if ((err & MCFG_OS_ERROR_MASK) == MCFG_OS_ERROR_MASK)
    return strerror(~MCFG_OS_ERROR_MASK & err);

  switch (err) {
  case MCFG_OK:
    return "Everything is OK :)";
  case MCFG_INVALID_PARSER_STATE:
    return "Invalid Parser State.";
  case MCFG_SYNTAX_ERROR:
    return "Syntax Error";
  case MCFG_INVALID_KEYWORD:
    return "Invalid Keyword/Token";
  default:
    return "invalid error code";
  }
}

struct _mcfg_token_id {
  char *name;
  mcfg_token_t value;
};

const struct _mcfg_token_id TOKEN_IDS[] = {
    {.name = "sector", .value = TOKEN_SECTOR},
    {.name = "section", .value = TOKEN_SECTION},
    {.name = "end", .value = TOKEN_END},
    {.name = ";", .value = TOKEN_COMMENT},
    {.name = "comment", .value = TOKEN_COMMENT},
    {.name = "str", .value = TOKEN_STR},
    {.name = "list", .value = TOKEN_LIST},
    {.name = "bool", .value = TOKEN_BOOL},
    {.name = "i8", .value = TOKEN_I8},
    {.name = "u8", .value = TOKEN_U8},
    {.name = "i16", .value = TOKEN_I16},
    {.name = "u16", .value = TOKEN_U16},
    {.name = "i32", .value = TOKEN_I32},
    {.name = "u32", .value = TOKEN_U32},
};
const size_t EXISTING_TOKEN_COUNT =
    sizeof(TOKEN_IDS) / sizeof(struct _mcfg_token_id);

uint8_t string_empty(char *in) {
  if (in == NULL || in[0] == 0)
    return 0;

  size_t len = strlen(in);
  for (size_t i = 0; i < len; i++)
    if (in[i] > ' ')
      return 1;

  return 0;
}

mcfg_token_t mcfg_get_token(char *in, uint16_t index) {
  if (string_empty(in) == 0)
    return TOKEN_EMPTY;
  mcfg_token_t tok = TOKEN_INVALID;

  in = strdup(in);
  char *string_tok_ptr;
  char *string_tok = strtok_r(in, " ", &string_tok_ptr);

  uint16_t current_index = 0;
  while (string_tok != NULL) {
    if (current_index == 0)
      break;
  }

  if (string_tok == NULL)
    goto mcfg_get_token_exit;

  for (size_t ix = 0; ix < EXISTING_TOKEN_COUNT; ix++) {
    if (strcmp(string_tok, TOKEN_IDS[ix].name) == 0) {
      tok = TOKEN_IDS[ix].value;
      break;
    }
  }

mcfg_get_token_exit:
  free(in);
  return tok;
}

mcfg_err_t _parse_outside_sector(char *line, mcfg_parser_ctxt_t *ctxt) {
  mcfg_token_t tok = mcfg_get_token(line, 0);
  if (tok == TOKEN_INVALID)
    return MCFG_INVALID_KEYWORD;

  if (tok == TOKEN_EMPTY)
    return MCFG_OK;

  return MCFG_OK;
}

mcfg_err_t _parse_sector(char *line, mcfg_parser_ctxt_t *ctxt) {
  return MCFG_OK;
}

mcfg_err_t _parse_section(char *line, mcfg_parser_ctxt_t *ctxt) {
  return MCFG_OK;
}

mcfg_err_t _parse_field(char *line, mcfg_parser_ctxt_t *ctxt) {
  return MCFG_OK;
}

mcfg_err_t mcfg_parse_line(char *line, mcfg_parser_ctxt_t *ctxt) {
  if (ctxt->target_file == NULL)
    return MCFG_INVALID_PARSER_STATE;

  if (ctxt->target_sector == NULL)
    return _parse_outside_sector(line, ctxt);

  if (ctxt->target_section == NULL)
    return _parse_sector(line, ctxt);

  if (ctxt->target_field == NULL)
    return _parse_section(line, ctxt);
  else
    return _parse_field(line, ctxt);

  return MCFG_INVALID_PARSER_STATE;
}

mcfg_err_t mcfg_parse_file_ctxto(char *path, mcfg_file_t *file, 
                           mcfg_parser_ctxt_t **ctxt_out) {
  FILE *in_file;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  file->sector_count = 0;
  file->dynfield_count = 0;

  mcfg_parser_ctxt_t ctxt = {
      .target_file = file,
      .target_sector = NULL,
      .target_section = NULL,
      .target_field = NULL,
      .linenum = 0,
      .file_path = path,
  };

  if (ctxt_out != NULL)
    *ctxt_out = &ctxt;

  errno = 0;
  in_file = fopen(path, "r");
  if (in_file == NULL)
    return MCFG_OS_ERROR_MASK | errno;
  
  mcfg_err_t result = MCFG_OK;
  while ((read = getline(&line, &len, in_file)) != -1) {
    ctxt.linenum++;
    result = mcfg_parse_line(line, &ctxt);
    if (result != MCFG_OK)
      break;
  }

  fclose(in_file);
  if (line)
    free(line);

  return result;
}

mcfg_err_t mcfg_parse_file(char *path, mcfg_file_t *file) {
  return mcfg_parse_file_ctxto(path, file, NULL);
}

void mcfg_free_field(mcfg_field_t *field) {}

void mcfg_free_section(mcfg_section_t *section) {}

void mcfg_free_sector(mcfg_sector_t *sector) {}

void mcfg_free_file(mcfg_file_t *file) {}
