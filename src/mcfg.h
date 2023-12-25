#ifndef MCFG_H
#define MCFG_H

#include <stddef.h>
#include <stdint.h>

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
  void *data;
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

typedef struct mcfg_parser_ctxt_t {
  mcfg_file_t *target_file;
  mcfg_sector_t *target_sector;
  mcfg_section_t *target_section;
  mcfg_field_t *target_field;

  uint32_t linenum;
  char *file_path;
} mcfg_parser_ctxt_t;

char *mcfg_err_string(mcfg_err_t err);

mcfg_err_t mcfg_parse_line(char *line, mcfg_parser_ctxt_t *ctxt);

mcfg_err_t mcfg_parse_file(char *path, mcfg_file_t *file);

void mcfg_free_field(mcfg_field_t *field);

void mcfg_free_section(mcfg_section_t *section);

void mcfg_free_sector(mcfg_sector_t *sector);

void mcfg_free_file(mcfg_file_t *file);

#endif // ifndef MCFG_H
