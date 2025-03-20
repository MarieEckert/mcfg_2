# MCFG/2 serializer usage
## Introduction
Since version 0.5.0, MCFG/2 provides an API for serializing the provided data structures into
text.

## Overview

The primary public function for serialization in `mcfg_serialize` which takes the `mcfg_file_t` to
serialize as well as options for serialization in the form of the `mcfg_serialize_options_t` struct.
```c
typedef struct mcfg_serialize_options {
	bool tab_indentation;
	int space_count;
} mcfg_serialize_options_t;

mcfg_serialize_result_t mcfg_serialize(mcfg_file_t file,
									   mcfg_serialize_options_t options);
```

This function returns an `mcfg_serialize_result_t` struct.
```c
typedef struct mcfg_serialize_result {
	/** @brief The error that occured whilst serializing, MCFG_OK on success. */
	mcfg_err_t err;

	/** @brief The serialized data. */
	mcfg_string_t *value;
} mcfg_serialize_result_t;
```

On success the `err` field will be set to MCFG_OK and the serialized data is set in `value`. This can be
accessed as a C-String by accessing `value->data`. The mcfg_string_t struct was also introduced in version
0.5.0 and represents a dynamic pascal-style string. Utility functions for working with this can be found in
`mcfg_util`.

```c
typedef struct mcfg_string {
	/**
	 * @brief total amount of space within data (including last-byte used as
	 * NULL terminator)
	 */
	uint64_t capacity;

	/**
	 * @brief used amount of space within data (not including NULL terminator)
	 */
	uint64_t length;

	char data[];
} mcfg_string_t;
```

## Usage Example

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcfg.h"
#include "mcfg_util.h"

int main(void) {
	mcfg_file_t file = /* ... get data into here, see the parser example ... */;

	mcfg_serialize_result_t serialized_result =
		mcfg_serialize(file, MCFG_DEFAULT_SERIALIZE_OPTIONS);
	if(serialized_result.err != MCFG_OK) {
		fprintf(stderr, STEP_LOG_PRIMER "mcfg serialization failed: %s (%d)\n",
				mcfg_err_string(serialized_result.err), serialized_result.err);
		exit(current_step);
	}

	printf(serialized_result.value->data);

  return 0;
}
```