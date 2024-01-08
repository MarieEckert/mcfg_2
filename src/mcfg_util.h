// mcfg_util.h ; marie config format utility header
// for mcfg version 2
//
// Copyright (c) 2023, Marie Eckert
// Licensend under the BSD 3-Clause License.
//------------------------------------------------------------------------------

#ifndef MCFG_UTIL_H
#define MCFG_UTIL_H

#include "mcfg.h"

//------------------------------------------------------------------------------
// Converts the data of the given field to a string representation
//
// Params:
// field The field of which the data should be converted
//
// Returns:
// Heap-Allocated string representation of the fields data.
char *mcfg_data_to_string(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as a string.
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// A char-pointer to the data variable in the given field. If the field's type
// is not TYPE_STRING a NULL-Pointer will be returned.
char *mcfg_data_as_string(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as an int.
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as an integer. If the type is not of
// a number variant (boolean, u8, i8, ...) or the data is a NULL-Pointer,
// 0 will be returned as default.
int mcfg_data_as_int(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as a boolean
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as a boolean (mcfg_boolean_t). If
// the type is not boolean or the data is a NULL-Pointer,
// false will be returned as default.
mcfg_boolean_t mcfg_data_as_bool(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as an u8
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as an uint8_t. If the field's type
// is not TYPE_U8 or the data is a NULL-Pointer, 0 will be returned as default.
uint8_t mcfg_data_as_u8(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as an i8
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as an int8_t. If the field's type
// is not TYPE_I8 or the data is a NULL-Pointer, 0 will be returned as default.
int8_t mcfg_data_as_i8(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as an u16
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as an uint16_t. If the field's type
// is not TYPE_U16 or the data is a NULL-Pointer, 0 will be returned as default.
uint16_t mcfg_data_as_u16(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as an i16
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as an int16_t. If the field's type
// is not TYPE_I16 or the data is a NULL-Pointer, 0 will be returned as default.
int16_t mcfg_data_as_i16(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as an u32
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as an uint32_t. If the field's type
// is not TYPE_U32 or the data is a NULL-Pointer, 0 will be returned as default.
uint32_t mcfg_data_as_u32(mcfg_field_t field);

//------------------------------------------------------------------------------
// Get the data of the field as an i32
//
// Params:
// field The field of which the data should be grabbed
//
// Returns:
// The value of the fields data represented as an int32_t. If the field's type
// is not TYPE_I32 or the data is a NULL-Pointer, 0 will be returned as default.
int32_t mcfg_data_as_i32(mcfg_field_t field);

#endif // ifndef MCFG_UTIL_H
