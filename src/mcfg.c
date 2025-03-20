/* mcfg.c ; marie config format parser implementation
 * implementation for MCFG/2
 *
 * Copyright (c) 2023, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

/* TODO:
 * - Parse lists with element type string
 * - Edge-Case Handling (checking for null-pointer, ...)
 */

#define _XOPEN_SOURCE	700
#define _POSIX_C_SOURCE 2

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcfg.h"
#include "shared.h"

#define XMALLOC(s)                   \
	({                               \
		void *ret = malloc(s);       \
		if(ret == NULL) {            \
			return MCFG_MALLOC_FAIL; \
		}                            \
		ret;                         \
	})

#define XREALLOC(o, s)               \
	({                               \
		void *ret = realloc(o, s);   \
		if(ret == NULL) {            \
			return MCFG_MALLOC_FAIL; \
		}                            \
		ret;                         \
	})

char *
mcfg_err_string(mcfg_err_t err)
{
	if((err & MCFG_OS_ERROR_MASK) == MCFG_OS_ERROR_MASK) {
		return strerror(~MCFG_OS_ERROR_MASK & err);
	}

	switch(err) {
		case MCFG_TODO:
			return "TODO";
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
		case MCFG_DUPLICATE_FIELD:
			return "Duplicate Field";
		case MCFG_DUPLICATE_DYNFIELD:
			return "Duplicate Dynamic Field";
		case MCFG_INVALID_TYPE:
			return "Invalid Datatype";
		case MCFG_NULLPTR:
			return "NULL-Pointer";
		case MCFG_INTEGER_OUT_OF_BOUNDS:
			return "Integer value is out of bounds";
		case MCFG_MALLOC_FAIL:
			return "A memory (re)allocation failed!";
		default:
			return "invalid error code";
	}
}

ssize_t
mcfg_sizeof(mcfg_field_type_t type)
{
	switch(type) {
		case TYPE_BOOL:
		case TYPE_I8:
		case TYPE_U8:
			return 1;
		case TYPE_I16:
		case TYPE_U16:
			return 2;
		case TYPE_I32:
		case TYPE_U32:
			return 4;
		default:
			return -1;
	}
}

mcfg_err_t
mcfg_add_sector(mcfg_file_t *file, char *name)
{
	if(file == NULL) {
		return MCFG_NULLPTR;
	}

	size_t ix = file->sector_count;

	remove_newline(name);
	if(file->sector_count == 0) {
		file->sectors = XMALLOC(sizeof(*file->sectors));
	} else {
		if(mcfg_get_sector(file, name) != NULL) {
			return MCFG_DUPLICATE_SECTOR;
		}
		file->sectors = XREALLOC(
			file->sectors, sizeof(mcfg_sector_t) * (file->sector_count + 1));
	}

	file->sectors[ix].name = name;
	file->sectors[ix].section_count = 0;
	file->sector_count++;
	return MCFG_OK;
}

mcfg_err_t
mcfg_add_section(mcfg_sector_t *sector, char *name)
{
	if(sector == NULL) {
		return MCFG_NULLPTR;
	}

	size_t ix = sector->section_count;

	remove_newline(name);
	if(sector->section_count == 0) {
		sector->sections = XMALLOC(sizeof(*sector->sections));
	} else {
		if(mcfg_get_section(sector, name) != NULL) {
			return MCFG_DUPLICATE_FIELD;
		}
		sector->sections =
			XREALLOC(sector->sections,
					 sizeof(mcfg_section_t) * (sector->section_count + 1));
	}

	sector->sections[ix].name = name;
	sector->sections[ix].field_count = 0;
	sector->section_count++;
	return MCFG_OK;
}

mcfg_err_t
mcfg_add_dynfield(mcfg_file_t *file,
				  mcfg_field_type_t type,
				  char *name,
				  void *data,
				  size_t size)
{
	if(file == NULL) {
		return MCFG_NULLPTR;
	}

	size_t ix = file->dynfield_count;

	remove_newline(name);
	if(file->dynfield_count == 0) {
		file->dynfields = XMALLOC(sizeof(*file->dynfields));
	} else {
		if(mcfg_get_dynfield(file, name) != NULL) {
			return MCFG_DUPLICATE_DYNFIELD;
		}
		file->dynfields =
			XREALLOC(file->dynfields,
					 sizeof(*file->dynfields) * (file->dynfield_count + 1));
	}

	file->dynfields[ix].type = type;
	file->dynfields[ix].name = name;
	file->dynfields[ix].data = data;
	file->dynfields[ix].size = size;
	file->dynfield_count++;
	return MCFG_OK;
}

mcfg_err_t
mcfg_add_field(mcfg_section_t *section,
			   mcfg_field_type_t type,
			   char *name,
			   void *data,
			   size_t size)
{
	if(section == NULL) {
		return MCFG_NULLPTR;
	}

	size_t ix = section->field_count;

	remove_newline(name);
	if(section->field_count == 0) {
		section->fields = XMALLOC(sizeof(*section->fields));
	} else {
		if(mcfg_get_field(section, name) != NULL) {
			return MCFG_DUPLICATE_FIELD;
		}
		section->fields = XREALLOC(
			section->fields, sizeof(mcfg_field_t) * (section->field_count + 1));
	}

	section->fields[ix].type = type;
	section->fields[ix].name = name;
	section->fields[ix].data = data;
	section->fields[ix].size = size;
	section->field_count++;
	return MCFG_OK;
}

mcfg_err_t
mcfg_add_list_field(mcfg_list_t *list, size_t size, void *data)
{
	if(list == NULL || data == NULL) {
		return MCFG_NULLPTR;
	}

	char *name = malloc((size_t)floor(log10((double)SIZE_MAX)));
	sprintf(name, "%zu", list->field_count + 1);

	size_t ix = list->field_count;

	if(list->field_count == 0) {
		list->fields = XMALLOC(sizeof(*list->fields));
	} else {
		list->fields = XREALLOC(list->fields,
								sizeof(mcfg_field_t) * (list->field_count + 1));
	}

	list->fields[ix].type = list->type;
	list->fields[ix].name = name;
	list->fields[ix].data = data;
	list->fields[ix].size = size;
	list->field_count++;
	return MCFG_OK;
}

mcfg_sector_t *
mcfg_get_sector(mcfg_file_t *file, char *name)
{
	mcfg_sector_t *ret = NULL;

	for(size_t ix = 0; ix < file->sector_count; ix++) {
		if(strcmp(file->sectors[ix].name, name) == 0) {
			ret = &file->sectors[ix];
			break;
		}
	}

	return ret;
}

mcfg_section_t *
mcfg_get_section(mcfg_sector_t *sector, char *name)
{
	mcfg_section_t *ret = NULL;

	for(size_t ix = 0; ix < sector->section_count; ix++) {
		if(strcmp(sector->sections[ix].name, name) == 0) {
			ret = &sector->sections[ix];
			break;
		}
	}

	return ret;
}

mcfg_field_t *
mcfg_get_dynfield(mcfg_file_t *file, char *name)
{
	mcfg_field_t *ret = NULL;

	for(size_t ix = 0; ix < file->dynfield_count; ix++) {
		if(strcmp(file->dynfields[ix].name, name) == 0) {
			ret = &file->dynfields[ix];
			break;
		}
	}

	return ret;
}

mcfg_field_t *
mcfg_get_field(mcfg_section_t *section, char *name)
{
	mcfg_field_t *ret = NULL;

	for(size_t ix = 0; ix < section->field_count; ix++) {
		if(strcmp(section->fields[ix].name, name) == 0) {
			ret = &section->fields[ix];
			break;
		}
	}

	return ret;
}

void
mcfg_free_list(mcfg_list_t list)
{
	for(size_t ix = 0; ix < list.field_count; ix++) {
		mcfg_free_field(list.fields[ix]);
	}

	free(list.fields);
}

void
mcfg_free_field(mcfg_field_t field)
{
	if(field.name != NULL) {
		free(field.name);
	}

	if(field.data != NULL) {
		if(field.type == TYPE_LIST) {
			mcfg_free_list(*(mcfg_list_t *)field.data);
		}

		free(field.data);
	}
}

void
mcfg_free_section(mcfg_section_t section)
{
	if(section.field_count > 0 && section.fields != NULL) {
		for(size_t ix = 0; ix < section.field_count; ix++) {
			mcfg_free_field(section.fields[ix]);
		}
	}

	if(section.fields != NULL) {
		free(section.fields);
	}

	if(section.name != NULL) {
		free(section.name);
	}
}

void
mcfg_free_sector(mcfg_sector_t sector)
{
	if(sector.section_count > 0 && sector.sections != NULL) {
		for(size_t ix = 0; ix < sector.section_count; ix++) {
			mcfg_free_section(sector.sections[ix]);
		}
	}

	if(sector.sections != NULL) {
		free(sector.sections);
	}

	if(sector.name != NULL) {
		free(sector.name);
	}
}

void
mcfg_free_file(mcfg_file_t file)
{
	if(file.dynfield_count > 0 && file.dynfields != NULL) {
		for(size_t ix = 0; ix < file.dynfield_count; ix++) {
			mcfg_free_field(file.dynfields[ix]);
		}
	}

	if(file.dynfields != NULL) {
		free(file.dynfields);
	}

	if(file.sector_count > 0 && file.sectors != NULL) {
		for(size_t ix = 0; ix < file.sector_count; ix++) {
			mcfg_free_sector(file.sectors[ix]);
		}
	}

	if(file.sectors != NULL) {
		free(file.sectors);
	}
}

/* parser api */

#include "parse.h"

mcfg_parse_result_t
mcfg_parse(char *input)
{
	mcfg_parse_result_t result = {
		.err = MCFG_OK,
		.err_linespan = {.starting_line = 0, .line_count = 0},
		.value = {0},
	};

	syntax_tree_t *tree = malloc(sizeof(syntax_tree_t));
	if(tree == NULL) {
		result.err = MCFG_MALLOC_FAIL;
		return result;
	}

	result.err = lex_input(input, tree);
	if(result.err != MCFG_OK) {
		free_tree(tree);
		return result;
	}

	_parse_result_t parse_result = parse_tree(*tree, &result.value);
	result.err = parse_result.err;
	result.err_linespan = parse_result.err_linespan;

	if(result.err != MCFG_OK) {
		mcfg_free_file(result.value);
	}

	free_tree(tree);

	return result;
}

mcfg_parse_result_t
mcfg_parse_from_file(const char *path)
{
	mcfg_parse_result_t result = {
		.err = MCFG_OK,
		.err_linespan = {.starting_line = 0, .line_count = 0},
		.value = {0},
	};

	FILE *raw_file = fopen(path, "rb");
	if(raw_file == NULL) {
		result.err = errno | MCFG_OS_ERROR_MASK;
		return result;
	}

	fseek(raw_file, 0, SEEK_END);
	const size_t data_size = ftell(raw_file);
	rewind(raw_file);

	char *data = malloc(data_size + 1);
	if(data == NULL) {
		fclose(raw_file);

		result.err = MCFG_MALLOC_FAIL;
		return result;
	}

	if(fread(data, data_size, 1, raw_file) != 1) {
		fclose(raw_file);

		result.err = errno | MCFG_OS_ERROR_MASK;
		return result;
	}

	fclose(raw_file);

	data[data_size] = 0;
	result = mcfg_parse(data);
	free(data);

	return result;
}

/* serializer api */

#include "serialize.h"

mcfg_serialize_result_t
mcfg_serialize(mcfg_file_t file, mcfg_serialize_options_t options)
{
	return serialize_file(file, options);
}