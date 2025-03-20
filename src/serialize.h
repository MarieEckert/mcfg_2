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

#endif