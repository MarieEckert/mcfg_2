#ifndef MCFG_H
#define MCFG_H

#include <stddef.h>
#include <stdint.h>

typedef enum mcfg_err {
  MCFG_TODO = -1,
  MCFG_OK,
  MCFG_INVALID_PARSER_STATE,
  MCFG_SYNTAX_ERROR,
  MCFG_INVALID_KEYWORD,
  MCFG_END_IN_NOWHERE,
  MCFG_STRUCTURE_ERROR,
  MCFG_DUPLICATE_SECTOR,
  MCFG_DUPLICATE_SECTION,
  MCFG_NULLPTR,
  MCFG_OS_ERROR_MASK = 0xf000
} mcfg_err_t;

typedef enum mcfg_field_type {
  TYPE_INVALID,
  TYPE_STRING,
  TYPE_LIST,
  TYPE_BOOL,
  TYPE_I8,
  TYPE_U8,
  TYPE_I16,
  TYPE_U16,
  TYPE_I32,
  TYPE_U32,
} mcfg_field_type_t;

typedef struct mcfg_field {
  char *name;
  mcfg_field_type_t type;
  void *data;
  size_t length;
} mcfg_field_t;

typedef struct mcfg_section {
  char *name;
  size_t field_count;
  mcfg_field_t *fields;
} mcfg_section_t;

typedef struct mcfg_sector {
  char *name;
  size_t section_count;
  mcfg_section_t *sections;
} mcfg_sector_t;

typedef struct mcfg_file {
  size_t sector_count;
  mcfg_sector_t *sectors;

  size_t dynfield_count;
  mcfg_field_t *dynfields;
} mcfg_file_t;

typedef struct mcfg_parser_ctxt {
  mcfg_file_t *target_file;
  mcfg_sector_t *target_sector;
  mcfg_section_t *target_section;
  mcfg_field_t *target_field;

  uint32_t linenum;
  char *file_path;
} mcfg_parser_ctxt_t;

typedef struct mcfg_data_parse_result {
  mcfg_err_t error;

  int multiline;
  void *data;
} mcfg_data_parse_result_t;

typedef enum mcfg_token {
  TOKEN_INVALID = -1,
  TOKEN_SECTOR,
  TOKEN_SECTION,
  TOKEN_END,
  TOKEN_COMMENT,
  TOKEN_STR,
  TOKEN_LIST,
  TOKEN_BOOL,
  TOKEN_I8,
  TOKEN_U8,
  TOKEN_I16,
  TOKEN_U16,
  TOKEN_I32,
  TOKEN_U32,
  TOKEN_EMPTY,
} mcfg_token_t;

char *mcfg_err_string(mcfg_err_t err);

mcfg_field_type_t mcfg_str_to_type(char *strtype);

mcfg_token_t mcfg_get_token(char *in, uint16_t index);

mcfg_data_parse_result_t mcfg_parse_field_data(mcfg_field_type_t type,
                                               char *str);

mcfg_err_t mcfg_parse_line(char *line, mcfg_parser_ctxt_t *ctxt);

mcfg_err_t mcfg_parse_file_ctxto(char *path, mcfg_file_t *file,
                                 mcfg_parser_ctxt_t **ctxt_out);
mcfg_err_t mcfg_parse_file(char *path, mcfg_file_t *file);

mcfg_err_t mcfg_add_sector(mcfg_file_t *file, char *name);

mcfg_err_t mcfg_add_section(mcfg_sector_t *sector, char *name);

mcfg_err_t mcfg_add_field(mcfg_section_t *section, mcfg_field_type_t type,
                          char *name, void *data);

mcfg_sector_t *mcfg_get_sector(mcfg_file_t *file, char *name);

mcfg_section_t *mcfg_get_section(mcfg_sector_t *sector, char *name);

mcfg_field_t *mcfg_get_field(mcfg_section_t *section, char *name);

void mcfg_free_field(mcfg_field_t *field);

void mcfg_free_section(mcfg_section_t *section);

void mcfg_free_sector(mcfg_sector_t *sector);

void mcfg_free_file(mcfg_file_t *file);

#endif // ifndef MCFG_H
