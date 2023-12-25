#ifndef MCFG_H
#define MCFG_H

#include <stddef.h>

typedef enum mcfg_err_t {
  MCFG_OK,
  MCFG_SYNTAX_ERROR,
  MCFG_OS_ERROR_MASK = 0xf000
} mcfg_err_t;

typedef enum mcfg_field_type_t {
  STRING,
  LIST
} mcfg_field_type_t;

typedef struct mcfg_field_t {
  char *name;
  mcfg_field_type_t type;
  char *data;
  size_t length;
} mcfg_field_t;

typedef struct mcfg_section_t {
  char *name;
  size_t field_count;
  mcfg_field_t *fields;
} mcfg_section_t;

typedef struct mcfg_sector_t {
  char *name;
  size_t section_count;
  mcfg_section_t *sections;
} mcfg_sector_t;

typedef struct mcfg_file_t {
  size_t sector_count;
  mcfg_sector_t *sectors;
} mcfg_file_t;

char *mcfg_err_string(mcfg_err_t err);

mcfg_err_t mcfg_parse_line(char *line, size_t len, mcfg_file_t *file);

mcfg_err_t mcfg_parse(char *input, size_t size, mcfg_file_t *file);

mcfg_err_t mcfg_parse_file(char *path, mcfg_file_t *out);

#endif // ifndef MCFG_H
