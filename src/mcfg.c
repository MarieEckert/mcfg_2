// mcfg.c ; marie config format parser implementation
// implementation for mcfg version 2
//
// Copyright (c) 2023, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#include "mcfg.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

char *mcfg_err_string(mcfg_err_t err) {
  return "ficken";
}

mcfg_err_t mcfg_parse_line(char *line, mcfg_parser_ctxt_t *ctxt) {
  printf("%s\n", line);
  return MCFG_OK;
}

mcfg_err_t mcfg_parse_file(char *path, mcfg_file_t *file) {
  FILE *in_file;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  errno = 0;
  in_file = fopen(path, "r");
  if (in_file == NULL)
    return MCFG_OS_ERROR_MASK | errno;

  file->sector_count = 0;

  mcfg_parser_ctxt_t ctxt = {
    .target_file = file,
    .target_sector = NULL,
    .target_section = NULL,
    .target_field = NULL,
    .linenum = 0,
    .file_path = path,
  };
  
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

void mcfg_free_field(mcfg_field_t *field) {
}

void mcfg_free_section(mcfg_section_t *section) {
}

void mcfg_free_sector(mcfg_sector_t *sector) {
}

void mcfg_free_file(mcfg_file_t *file) {
}
