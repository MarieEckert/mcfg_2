// mcfg_util.h ; marie config format utility header
// for mcfg version 2
//
// Copyright (c) 2023, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#ifndef MCFG_UTIL_H
#define MCFG_UTIL_H

#include "mcfg.h"

char *mcfg_data_to_string(mcfg_field_t field);

char *mcfg_data_as_string(mcfg_field_t field);

int mcfg_data_as_int(mcfg_field_t field);

mcfg_boolean_t mcfg_data_as_bool(mcfg_field_t field);

uint8_t mcfg_data_as_u8(mcfg_field_t field);

int8_t mcfg_data_as_i8(mcfg_field_t field);

uint16_t mcfg_data_as_u16(mcfg_field_t field);

int16_t mcfg_data_as_i16(mcfg_field_t field);

uint32_t mcfg_data_as_u32(mcfg_field_t field);

int32_t mcfg_data_as_i32(mcfg_field_t field);

#endif // ifndef MCFG_UTIL_H
