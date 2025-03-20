/* serialize.c ; marie config format internal serializer implementation
 * implementation for MCFG/2
 *
 * Copyright (c) 2025, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#define _XOPEN_SOURCE	700
#define _POSIX_C_SOURCE 2

#include "cptrlist.h"
#include <stdlib.h>
#include <string.h>

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
#define ERR_CHECK(e)         \
	({                       \
		mcfg_err_t _e = e;   \
		if(_e != MCFG_OK) {  \
			ERR_NOTE(_e);    \
			result.err = _e; \
			return result;   \
		}                    \
	})

mcfg_serialize_result_t
serialize_file(mcfg_file_t file, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	CPtrList sector_strings;
	cptrlist_init(&sector_strings, 16, 16);

	size_t result_size = 0;

	for(size_t ix = 0; ix < file.sector_count; ix++) {
		result = serialize_sector(file.sectors[ix], options);
		if(result.err != MCFG_OK) {
			goto exit;
		}

		result_size += result.value->length;
		cptrlist_append(&sector_strings, result.value);
	}

	result.value = mcfg_string_new_sized(result_size + 1);
	if(result.value == NULL) {
		result.err = MCFG_MALLOC_FAIL;
		goto exit;
	}

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
	cptrlist_init(&section_strings, 16, 16);

	size_t result_size = 0;
	for(size_t ix = 0; ix < sector.section_count; ix++) {
		result = serialize_section(sector.sections[ix], options);
		if(result.err != MCFG_OK) {
			goto exit;
		}

		result_size += result.value->length;
		cptrlist_append(&section_strings, result.value);
	}

	result.value =
		mcfg_string_new_sized(result_size + sizeof(KEYWORD_SECTOR) +
							  sizeof(KEYWORD_END) + 1 + strlen(sector.name));
	if(result.value == NULL) {
		result.err = MCFG_MALLOC_FAIL;
		goto exit;
	}

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

	CPtrList field_strings;

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

		if(result.err != MCFG_OK) {
			goto exit;
		}

		result_size += result.value->length;
		cptrlist_append(&field_strings, result.value);
	}

	result.value =
		mcfg_string_new_sized(sizeof(KEYWORD_SECTION) + sizeof(KEYWORD_END) +
							  1 + strlen(section.name));
	if(result.value == NULL) {
		result.err = MCFG_MALLOC_FAIL;
		goto exit;
	}

	ERR_CHECK(mcfg_string_append_cstr(&result.value, "  " KEYWORD_SECTION " "));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, section.name));
	ERR_CHECK(mcfg_string_append_cstr(&result.value, "\n"));

	for(size_t ix = 0; ix < field_strings.size; ix++) {
		if(ix + 1 < section.field_count) {
			ERR_CHECK(mcfg_string_append_cstr(&result.value, "\n"));
		}
	}

	ERR_CHECK(mcfg_string_append_cstr(&result.value, "  " KEYWORD_END "\n"));

exit:
	if(result.err != MCFG_OK && result.value != NULL) {
		free(result.value);
	}
	return result;
}

mcfg_serialize_result_t
serialize_string_field(mcfg_field_t sector, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	return result;
}

mcfg_serialize_result_t
serialize_list_field(mcfg_field_t sector, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	return result;
}

mcfg_serialize_result_t
serialize_bool_field(mcfg_field_t sector, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	return result;
}

mcfg_serialize_result_t
serialize_number_field(mcfg_field_t sector, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	return result;
}