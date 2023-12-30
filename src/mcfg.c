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

uint8_t string_empty(char *in) {
  if (in == NULL || in[0] == 0)
    return 0;

  size_t len = strlen(in);
  for (size_t i = 0; i < len; i++)
    if (in[i] > ' ')
      return 1;

  return 0;
}

void remove_newline(char *in) {
  if (in[strlen(in) - 1] == '\n')
    in[strlen(in) - 1] = 0;
}

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
  case MCFG_END_IN_NOWHERE:
    return "Usage of \"end\" keyword in nowhere";
  case MCFG_STRUCTURE_ERROR:
    return "Invalid structure of input";
  case MCFG_DUPLICATE_SECTOR:
    return "Duplicate Sector";
  case MCFG_DUPLICATE_SECTION:
    return "Duplicate Section";
  default:
    return "invalid error code";
  }
}

struct _mcfg_type_id {
  char *name;
  mcfg_field_type_t value;
};

const struct _mcfg_type_id TYPE_IDS[] = {
    {.name = "str", .value = TYPE_STRING}, {.name = "list", .value = TYPE_LIST},
    {.name = "bool", .value = TYPE_BOOL},  {.name = "i8", .value = TYPE_I8},
    {.name = "u8", .value = TYPE_U8},      {.name = "i16", .value = TYPE_I16},
    {.name = "u16", .value = TYPE_U16},    {.name = "i32", .value = TYPE_I32},
    {.name = "u32", .value = TYPE_U32},
};
const size_t EXISTING_TYPE_COUNT =
    sizeof(TYPE_IDS) / sizeof(struct _mcfg_type_id);

mcfg_field_type_t mcfg_str_to_type(char *strtype) {
  mcfg_field_type_t ret = TYPE_INVALID;

  strtype = strdup(strtype);
  remove_newline(strtype);

  if (string_empty(strtype) == 0) {
    goto mcfg_get_token_exit;
  }

  for (size_t ix = 0; ix < EXISTING_TYPE_COUNT; ix++) {
    if (strcmp(strtype, TYPE_IDS[ix].name) == 0) {
      ret = TYPE_IDS[ix].value;
      break;
    }
  }

mcfg_get_token_exit:
  free(strtype);
  return ret;
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

char *mcfg_get_token_raw(char *in, uint16_t index) {
  if (string_empty(in) == 0)
    return strdup("");

  char *string_tok_ptr = NULL;
  char *indup = strdup(in);
  char *ret = strtok_r(indup, " ", &string_tok_ptr);

  uint16_t current_index = 0;
  while (ret != NULL) {
    if (current_index == index)
      break;

    ret = strtok_r(NULL, " ", &string_tok_ptr);
    current_index++;
  }

  if (ret == NULL) {
    free(ret);
    return strdup("");
  }

  char *aret = strdup(ret);
  free(indup);
  return aret;
}

mcfg_token_t mcfg_get_token(char *in, uint16_t index) {
  mcfg_token_t tok = TOKEN_INVALID;
  in = mcfg_get_token_raw(in, index);

  remove_newline(in);

  if (string_empty(in) == 0) {
    tok = TOKEN_EMPTY;
    goto mcfg_get_token_exit;
  }

  for (size_t ix = 0; ix < EXISTING_TOKEN_COUNT; ix++) {
    if (strcmp(in, TOKEN_IDS[ix].name) == 0) {
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

  if (tok == TOKEN_EMPTY || tok == TOKEN_COMMENT)
    return MCFG_OK;

  if (tok == TOKEN_END)
    return MCFG_END_IN_NOWHERE;

  if (tok != TOKEN_SECTOR)
    return MCFG_STRUCTURE_ERROR;

  char *name = mcfg_get_token_raw(line, 1);
  mcfg_err_t ret = mcfg_add_sector(ctxt->target_file, name);

  if (ret != MCFG_OK) {
    free(name);
    return ret;
  }

  ctxt->target_sector =
      &ctxt->target_file->sectors[ctxt->target_file->sector_count - 1];

  return MCFG_OK;
}

mcfg_err_t _parse_sector(char *line, mcfg_parser_ctxt_t *ctxt) {
  mcfg_token_t tok = mcfg_get_token(line, 0);

  if (tok == TOKEN_INVALID)
    return MCFG_INVALID_KEYWORD;

  if (tok == TOKEN_EMPTY || tok == TOKEN_COMMENT)
    return MCFG_OK;

  if (tok == TOKEN_END) {
    ctxt->target_sector = NULL;
    return MCFG_OK;
  }

  if (tok != TOKEN_SECTION)
    return MCFG_STRUCTURE_ERROR;

  char *name = mcfg_get_token_raw(line, 1);
  mcfg_err_t ret = mcfg_add_section(ctxt->target_sector, name);

  if (ret != MCFG_OK) {
    free(name);
    return ret;
  }

  ctxt->target_section =
      &ctxt->target_sector->sections[ctxt->target_sector->section_count - 1];

  return MCFG_OK;
}

mcfg_err_t _parse_section(char *line, mcfg_parser_ctxt_t *ctxt) {
  mcfg_token_t tok = mcfg_get_token(line, 0);

  if (tok == TOKEN_INVALID)
    return MCFG_INVALID_KEYWORD;

  if (tok == TOKEN_EMPTY || tok == TOKEN_COMMENT)
    return MCFG_OK;

  if (tok == TOKEN_END) {
    ctxt->target_section = NULL;
    return MCFG_OK;
  }

  if (tok < TOKEN_STR)
    return MCFG_STRUCTURE_ERROR;

  mcfg_field_type_t type = mcfg_str_to_type(mcfg_get_token_raw(line, 1));
  char *name = mcfg_get_token_raw(line, 2);

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

mcfg_err_t mcfg_add_sector(mcfg_file_t *file, char *name) {
  size_t ix = file->sector_count++;

  if (file->sector_count == 1) {
    file->sectors = malloc(sizeof(mcfg_sector_t));
  } else {
    if (mcfg_get_sector(file, name) != NULL)
      return MCFG_DUPLICATE_SECTOR;
    file->sectors =
        realloc(file->sectors, sizeof(mcfg_sector_t) * file->sector_count);
  }

  file->sectors[ix].name = name;
  return MCFG_OK;
}

mcfg_err_t mcfg_add_section(mcfg_sector_t *sector, char *name) {
  size_t ix = sector->section_count++;

  if (sector->section_count == 1) {
    sector->sections = malloc(sizeof(mcfg_section_t));
  } else {
    if (mcfg_get_section(sector, name) != NULL)
      return MCFG_DUPLICATE_SECTION;
    sector->sections = realloc(sector->sections,
                               sizeof(mcfg_section_t) * sector->section_count);
  }

  sector->sections[ix].name = name;
  return MCFG_OK;
}

mcfg_err_t mcfg_add_field(mcfg_section_t *section, mcfg_field_type_t type,
                          char *name, void *data) {
  return MCFG_OK;
}

mcfg_sector_t *mcfg_get_sector(mcfg_file_t *file, char *name) {
  mcfg_sector_t *ret = NULL;

  for (size_t ix = 0; ix < file->sector_count; ix++) {
    if (strcmp(file->sectors[ix].name, name) == 0) {
      ret = &file->sectors[ix];
      break;
    }
  }

  return ret;
}

mcfg_section_t *mcfg_get_section(mcfg_sector_t *sector, char *name) {
  mcfg_section_t *ret = NULL;

  for (size_t ix = 0; ix < sector->section_count; ix++) {
    if (strcmp(sector->sections[ix].name, name) == 0) {
      ret = &sector->sections[ix];
      break;
    }
  }

  return ret;
}

mcfg_field_t *mcfg_get_field(mcfg_section_t *section, char *name) {
  mcfg_field_t *ret = NULL;

  for (size_t ix = 0; ix < section->field_count; ix++) {
    if (strcmp(section->fields[ix].name, name) == 0) {
      ret = &section->fields[ix];
      break;
    }
  }

  return ret;
}

void mcfg_free_field(mcfg_field_t *field) {}

void mcfg_free_section(mcfg_section_t *section) {}

void mcfg_free_sector(mcfg_sector_t *sector) {}

void mcfg_free_file(mcfg_file_t *file) {}
