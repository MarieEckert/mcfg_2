#ifndef MCFG_H
#define MCFG_H

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

#endif // ifndef MCFG_H
