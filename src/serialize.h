/* parse.h ; marie config format internal serializer header
 * for MCFG/2
 *
 * Copyright (c) 2025, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "mcfg.h"
#include "shared.h"

#define NAMESPACE	   serialize

#define serialize_file NAMESPACED_DECL(serialize_file)
mcfg_serialize_result_t serialize_file(mcfg_file_t file,
									   mcfg_serialize_options_t options);

#define serialize_sector NAMESPACED_DECL(serialize_sector)
mcfg_serialize_result_t serialize_sector(mcfg_sector_t sector,
										 mcfg_serialize_options_t options);

#define serialize_section NAMESPACED_DECL(serialize_section)
mcfg_serialize_result_t serialize_section(mcfg_section_t sector,
										  mcfg_serialize_options_t options);

#define serialize_string_field NAMESPACED_DECL(serialize_string_field)
mcfg_serialize_result_t serialize_string_field(
	mcfg_field_t sector,
	mcfg_serialize_options_t options);

#define serialize_list_field NAMESPACED_DECL(serialize_list_field)
mcfg_serialize_result_t serialize_list_field(mcfg_field_t sector,
											 mcfg_serialize_options_t options);

#define serialize_bool_field NAMESPACED_DECL(serialize_bool_field)
mcfg_serialize_result_t serialize_bool_field(mcfg_field_t sector,
											 mcfg_serialize_options_t options);

#define serialize_number_field NAMESPACED_DECL(serialize_number_field)
mcfg_serialize_result_t serialize_number_field(
	mcfg_field_t sector,
	mcfg_serialize_options_t options);

#endif