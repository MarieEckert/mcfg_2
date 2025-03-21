/* serialize.c ; marie config format internal serializer implementation
 * implementation for MCFG/2
 *
 * Copyright (c) 2025, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#define _XOPEN_SOURCE	700
#define _POSIX_C_SOURCE 2

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cptrlist.h"
#include "mcfg.h"
#include "mcfg_util.h"
#include "serialize.h"
#include "shared.h"

#define NAMESPACE		serialize

#define KEYWORD_SECTOR	"sector"
#define KEYWORD_SECTION "section"
#define KEYWORD_END		"end"
#define KEYWORD_LIST	"list"
#define KEYWORD_STR		"str"
#define KEYWORD_BOOL	"bool"
#define KEYWORD_I8		"i8"
#define KEYWORD_U8		"u8"
#define KEYWORD_I16		"i16"
#define KEYWORD_U16		"u16"
#define KEYWORD_I32		"i32"
#define KEYWORD_U32		"u32"

#undef ERR_CHECK

/**
 * Returns an error if the given value is not equal to MCFG_OK.
 * Works only in functions which return a struct called "result" which has an
 * mcfg_err_t field called err and a label called "exit" to jump to on error.
 */
#define ERR_CHECK(e)         \
	({                       \
		mcfg_err_t _e = e;   \
		if(_e != MCFG_OK) {  \
			ERR_NOTE(_e);    \
			result.err = _e; \
			goto exit;       \
		}                    \
	})

#define ASSERT_OR_RETURN(c, e) ERR_CHECK((c) ? MCFG_OK : e)

#define NULL_CHECK(p, e)	   ASSERT_OR_RETURN(p != NULL, e)

/* internal helper functions */

#define _make_indent			   NAMESPACED_DECL(_make_indent)
#define _serialize_string		   NAMESPACED_DECL(_serialize_string)
#define _number_serialization_func NAMESPACED_DECL(_number_serialization_func)
#define _bool_serialization_func   NAMESPACED_DECL(_bool_serialization_func)
#define _string_serialization_func NAMESPACED_DECL(_string_serialization_func)

/**
 * @brief Build a string used for indentation which is stored on the heap.
 * @return C-String for indentation, NULL if malloc fails.
 */
char *
_make_indent(mcfg_serialize_options_t options, int depth)
{
	if(options.tab_indentation) {
		char *ret = malloc(depth + 1);
		if(ret == NULL) {
			return ret;
		}

		memset(ret, '\t', depth);
		ret[depth] = 0;
		return ret;
	}

	const size_t total = options.space_count * depth + 1;
	char *ret = malloc(total);
	if(ret == NULL) {
		return ret;
	}

	memset(ret, ' ', total - 1);
	ret[total] = 0;
	return ret;
}

/**
 * @brief Converts a string into a valid MCFG/2 string (including opening and
 * closing quotes).
 */
mcfg_serialize_result_t
_serialize_string(mcfg_field_t field)
{
	mcfg_serialize_result_t result = {0};
	result.value = mcfg_string_new_sized(field.size + 2);
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	char *org = (char *)field.data;
	NULL_CHECK(org, MCFG_MALLOC_FAIL);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, "'"));

	char *current_pos = org;
	do {
		char *new_pos = strchrnul(current_pos, '\'');
		const bool hit = *new_pos == '\'';
		new_pos[0] = 0; /* set the position where strchrnul landed to NULL so
						   that we can be sure that mcfg_string_append_cstr only
						   appends the range we actually want to append*/

		ERR_CHECK(mcfg_string_append_cstr(&result.value, current_pos));

		if(hit) {
			ERR_CHECK(mcfg_string_append_cstr(&result.value, "''"));
		}

		current_pos = new_pos + 1;
	} while((size_t)(current_pos - org) < field.size);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, "'"));

exit:
	if(result.err != MCFG_OK && result.value != NULL) {
		free(result.value);
	}

	return result;
}

/**
 * @brief This type is primarly used to cleanup the logic in
 * serialize_list_field. These functions are currently not used
 * in any other serialize_..._field functions to avoid reallocations.
 * @see serialize_list_field
 */
typedef mcfg_err_t(list_serialization_func_t)(mcfg_field_t field,
											  mcfg_string_t **dest);

/**
 * @brief Generic serialization function for number types.
 * @see list_serialization_func_t
 */
mcfg_err_t
_number_serialization_func(mcfg_field_t field, mcfg_string_t **dest)
{
	char *temp = mcfg_data_to_string(field);
	mcfg_err_t result = mcfg_string_append_cstr(dest, temp);
	free(temp);
	return result;
}

/**
 * @brief Generic serialization function for booleans.
 * @see list_serialization_func_t
 */
mcfg_err_t
_bool_serialization_func(mcfg_field_t field, mcfg_string_t **dest)
{
	return mcfg_string_append_cstr(dest,
								   mcfg_data_as_bool(field) ? "true" : "false");
}

/**
 * @brief Generic serialization function for strings.
 * @see _serialize_string
 * @see list_serialization_func_t
 */
mcfg_err_t
_string_serialization_func(mcfg_field_t field, mcfg_string_t **dest)
{
	mcfg_serialize_result_t result = _serialize_string(field);
	ERR_CHECK(result.err);
	ERR_CHECK(mcfg_string_append(dest, result.value));

exit:
	if(result.value != NULL) {
		free(result.value);
	}
	return result.err;
}

/* serialize.h functions */

mcfg_serialize_result_t
serialize_file(mcfg_file_t file, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	CPtrList sector_strings;
	ASSERT_OR_RETURN(cptrlist_init(&sector_strings, 16, 16), MCFG_MALLOC_FAIL);

	size_t result_size = 0;

	for(size_t ix = 0; ix < file.sector_count; ix++) {
		result = serialize_sector(file.sectors[ix], options);
		if(result.err != MCFG_OK) {
			goto exit;
		}

		result_size += result.value->length;
		ASSERT_OR_RETURN(cptrlist_append(&sector_strings, result.value) >= 0,
						 MCFG_MALLOC_FAIL);
	}

	result.value = mcfg_string_new_sized(result_size + 1);
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	size_t copy_offset = 0;
	for(size_t ix = 0; ix < sector_strings.size; ix++) {
		mcfg_string_t *item = ((mcfg_string_t *)sector_strings.items[ix]);

		memcpy(result.value->data + copy_offset, item->data, item->length);
		copy_offset += item->length;
	}

	result.value->data[result_size] = 0;

exit:
	cptrlist_destroy(&sector_strings);
	return result;
}

mcfg_serialize_result_t
serialize_sector(mcfg_sector_t sector, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	CPtrList section_strings;
	ASSERT_OR_RETURN((cptrlist_init(&section_strings, 16, 16)),
					 MCFG_MALLOC_FAIL);

	size_t result_size = 0;
	for(size_t ix = 0; ix < sector.section_count; ix++) {
		result = serialize_section(sector.sections[ix], options);
		if(result.err != MCFG_OK) {
			goto exit;
		}

		result_size += result.value->length;
		ASSERT_OR_RETURN(cptrlist_append(&section_strings, result.value) >= 0,
						 MCFG_MALLOC_FAIL);
	}

	result.value =
		mcfg_string_new_sized(result_size + sizeof(KEYWORD_SECTOR) +
							  sizeof(KEYWORD_END) + 1 + strlen(sector.name));
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, KEYWORD_SECTOR " "));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, sector.name));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, "\n"));

	for(size_t ix = 0; ix < section_strings.size; ix++) {
		mcfg_string_t *item = ((mcfg_string_t *)section_strings.items[ix]);
		ERR_CHECK(mcfg_string_append(&result.value, item));

		if(ix + 1 < section_strings.size) {
			ERR_CHECK(mcfg_string_append_cstr(&result.value, "\n"));
		}
	}

	ERR_CHECK(mcfg_string_append_cstr(&result.value, KEYWORD_END "\n\n"));

exit:
	cptrlist_destroy(&section_strings);
	if(result.err != MCFG_OK && result.value != NULL) {
		free(result.value);
	}
	return result;
}

mcfg_serialize_result_t
serialize_section(mcfg_section_t section, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};
	char *indent = _make_indent(options, 1);
	CPtrList field_strings;

	ASSERT_OR_RETURN(cptrlist_init(&field_strings, 16, 16), MCFG_MALLOC_FAIL);
	NULL_CHECK(indent, MCFG_MALLOC_FAIL);

	size_t result_size = 0;
	for(size_t ix = 0; ix < section.field_count; ix++) {
		mcfg_field_t field = section.fields[ix];
		switch(field.type) {
			case TYPE_STRING:
				result = serialize_string_field(field, options);
				break;
			case TYPE_LIST:
				result = serialize_list_field(field, options);
				break;
			case TYPE_BOOL:
				result = serialize_bool_field(field, options);
				break;
			case TYPE_I8:
			case TYPE_U8:
			case TYPE_I16:
			case TYPE_U16:
			case TYPE_I32:
			case TYPE_U32:
				result = serialize_number_field(field, options);
				break;
			case TYPE_INVALID:
				result.err = MCFG_INVALID_TYPE;
				goto exit;
		}

		ERR_CHECK(result.err);

		if(result.value == NULL) {
			continue;
		}

		result_size += result.value->length;
		ASSERT_OR_RETURN(cptrlist_append(&field_strings, result.value) >= 0,
						 MCFG_MALLOC_FAIL);
	}

	result.value = mcfg_string_new_sized(
		result_size + strlen(indent) + sizeof(KEYWORD_SECTION) +
		sizeof(KEYWORD_END) + 1 + strlen(section.name));
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, indent));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, KEYWORD_SECTION " "));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, section.name));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, "\n"));

	for(size_t ix = 0; ix < field_strings.size; ix++) {
		mcfg_string_t *item = ((mcfg_string_t *)field_strings.items[ix]);
		ERR_CHECK(mcfg_string_append(&result.value, item));

		ERR_CHECK(mcfg_string_append_cstr(&result.value, "\n"));
	}

	ERR_CHECK(mcfg_string_append_cstr(&result.value, indent));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, KEYWORD_END "\n"));

exit:
	cptrlist_destroy(&field_strings);
	if(result.err != MCFG_OK && result.value != NULL) {
		free(result.value);
	}
	free(indent);

	return result;
}

mcfg_serialize_result_t
serialize_string_field(mcfg_field_t field, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	char *indent = _make_indent(options, 2);
	mcfg_serialize_result_t string_value = _serialize_string(field);
	ERR_CHECK(string_value.err);
	NULL_CHECK(indent, MCFG_MALLOC_FAIL);

	result.value = mcfg_string_new_sized(
		sizeof(KEYWORD_STR) + strlen(field.name) + string_value.value->length);
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, indent));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, KEYWORD_STR " "));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, field.name));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, " "));
	ERR_CHECK(mcfg_string_append(&result.value, string_value.value));

exit:
	if(result.err != MCFG_OK && result.value != NULL) {
		free(result.value);
	}

	if(string_value.value != NULL) {
		free(string_value.value);
	}

	if(indent != NULL) {
		free(indent);
	}

	return result;
}

mcfg_serialize_result_t
serialize_list_field(mcfg_field_t field, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	mcfg_list_t *list = mcfg_data_as_list(field);
	if(list == NULL) {
		result.err = MCFG_NULLPTR;
		return result;
	}

	char *datatype;
	list_serialization_func_t *serialize_func;

	switch(list->type) {
		case TYPE_I8:
			datatype = KEYWORD_LIST " " KEYWORD_I8 " ";
			serialize_func = &_number_serialization_func;
			break;
		case TYPE_U8:
			datatype = KEYWORD_LIST " " KEYWORD_U8 " ";
			serialize_func = &_number_serialization_func;
			break;
		case TYPE_I16:
			datatype = KEYWORD_LIST " " KEYWORD_I16 " ";
			serialize_func = &_number_serialization_func;
			break;
		case TYPE_U16:
			datatype = KEYWORD_LIST " " KEYWORD_U16 " ";
			serialize_func = &_number_serialization_func;
			break;
		case TYPE_I32:
			datatype = KEYWORD_LIST " " KEYWORD_I32 " ";
			serialize_func = &_number_serialization_func;
			break;
		case TYPE_U32:
			datatype = KEYWORD_LIST " " KEYWORD_U32 " ";
			serialize_func = &_number_serialization_func;
			break;
		case TYPE_BOOL:
			datatype = KEYWORD_LIST " " KEYWORD_BOOL " ";
			serialize_func = &_bool_serialization_func;
			break;
		case TYPE_STRING:
			datatype = KEYWORD_LIST " " KEYWORD_STR " ";
			serialize_func = &_string_serialization_func;
			break;
		default:
			result.err = MCFG_INVALID_TYPE;
			return result;
	}

	char *indent = _make_indent(options, 2);
	result.value = mcfg_string_new_sized(strlen(indent) + strlen(datatype) +
										 strlen(field.name) + 2);

	NULL_CHECK(indent, MCFG_MALLOC_FAIL);
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, indent));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, datatype));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, field.name));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, " "));

	for(size_t ix = 0; ix < list->field_count; ix++) {
		ERR_CHECK(serialize_func(list->fields[ix], &result.value));

		if(ix + 1 < list->field_count) {
			ERR_CHECK(mcfg_string_append_cstr(&result.value, ", "));
		}
	}

exit:
	if(result.err != MCFG_OK && result.value != NULL) {
		free(result.value);
	}

	if(indent != NULL) {
		free(indent);
	}

	return result;
}

mcfg_serialize_result_t
serialize_bool_field(mcfg_field_t field, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};
	char *indent = _make_indent(options, 2);
	NULL_CHECK(indent, MCFG_MALLOC_FAIL);

	const char *string_value = mcfg_data_as_bool(field) ? " true" : " false";

	result.value =
		mcfg_string_new_sized(strlen(indent) + sizeof(KEYWORD_BOOL) +
							  strlen(field.name) + strlen(string_value));
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, indent));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, KEYWORD_BOOL " "));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, field.name));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, string_value));

exit:
	free(indent);
	return result;
}

mcfg_serialize_result_t
serialize_number_field(mcfg_field_t field, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	char *string_value = mcfg_data_to_string(field);

	char *indent = _make_indent(options, 2);
	NULL_CHECK(indent, MCFG_MALLOC_FAIL);

	char *datatype;
	switch(field.type) {
		case TYPE_I8:
			datatype = KEYWORD_I8 " ";
			break;
		case TYPE_U8:
			datatype = KEYWORD_U8 " ";
			break;
		case TYPE_I16:
			datatype = KEYWORD_I16 " ";
			break;
		case TYPE_U16:
			datatype = KEYWORD_U16 " ";
			break;
		case TYPE_I32:
			datatype = KEYWORD_I32 " ";
			break;
		case TYPE_U32:
			datatype = KEYWORD_U32 " ";
			break;
		default:
			result.err = MCFG_INVALID_TYPE;
			goto exit;
	}

	result.value =
		mcfg_string_new_sized(strlen(indent) + strlen(datatype) +
							  strlen(field.name) + strlen(string_value) + 3);
	NULL_CHECK(result.value, MCFG_MALLOC_FAIL);

	ERR_CHECK(mcfg_string_append_cstr(&result.value, indent));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, datatype));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, field.name));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, " "));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, string_value));

exit:
	if(result.err != MCFG_OK) {
		if(result.value != NULL) {
			free(result.value);
		}
	}

	if(string_value != NULL) {
		free(string_value);
	}

	free(indent);

	return result;
}
