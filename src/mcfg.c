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
    default:
      return "invalid error code"; 
  }
}

mcfg_err_t _parse_outside_sector(char *line, mcfg_parser_ctxt_t *ctxt) {
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

mcfg_err_t mcfg_parse_file(char *path, mcfg_file_t *file) {
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

  errno = 0;
  in_file = fopen(path, "r");
  if (in_file == NULL)
    return MCFG_OS_ERROR_MASK | errno;

  while ((read = getline(&line, &len, in_file)) != -1) {
    ctxt.linenum++;
    mcfg_err_t result = mcfg_parse_line(line, &ctxt);
    if (result != MCFG_OK)
      return result;
  }

  fclose(in_file);
  if (line)
    free(line);

  return MCFG_OK;
}

void mcfg_free_field(mcfg_field_t *field) {}

void mcfg_free_section(mcfg_section_t *section) {}

void mcfg_free_sector(mcfg_sector_t *sector) {}

void mcfg_free_file(mcfg_file_t *file) {}
