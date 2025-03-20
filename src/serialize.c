/* parse.c ; marie config format internal serializer implementation
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

#include "serialize.h"
#include "shared.h"

#define NAMESPACE serialize

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

	result.value = malloc(sizeof(*result.value) +
						  sizeof(*result.value->data) * result_size);
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

exit:
	cptrlist_destroy(&sector_strings);
	return result;
}

mcfg_serialize_result_t
serialize_sector(mcfg_sector_t sector, mcfg_serialize_options_t options)
{
	mcfg_serialize_result_t result = {0};

	return result;
}