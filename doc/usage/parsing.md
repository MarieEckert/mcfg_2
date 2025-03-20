# MCFG/2 parser usage
## Introduction
The parser API of the MCFG/2 library is probably the most important interface,
providing the capabiliy to parse text from either a C-String or a file into the
provided data structures.

## Usage Example
### Parsing from file

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcfg.h"
#include "mcfg_util.h"

int main(void) {
  char path[] = "tests/number_types.mcfg";

  mcfg_parse_result_t ret = mcfg_parse_from_file(path);
  if (ret.err != MCFG_OK) {
	fprintf(stderr, "mcfg parsing failed: %s (%d)\n", mcfg_err_string(ret.err),
			ret.err);
	fprintf(stderr, "in file \"%s\" on line %zu\n", path,
			ret.err_linespan.starting_line);
	return 1;
  }

  mcfg_file_t file = ret.value;

  mcfg_field_t *field = mcfg_get_field_by_path(
	  &file, mcfg_parse_path("/test/numbers/u32_12312312"));
  printf("value = %lu\n", mcfg_data_as_u32(*field));

  return 0;
}
```

#### Parsing from a C-Style string

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcfg.h"
#include "mcfg_util.h"

char *data = /* ... */;

int main(void) {
	mcfg_parse_result_t ret = mcfg_parse(data);
	if (ret.err != MCFG_OK) {
		fprintf(stderr, "mcfg parsing failed: %s (%d)\n", mcfg_err_string(ret.err),
				ret.err);
		fprintf(stderr, "in file on line %zu\n", ret.err_linespan.starting_line);
		return 1;
	}

	mcfg_file_t file = ret.value;

	mcfg_field_t *field = mcfg_get_field_by_path(
		&file, mcfg_parse_path("/test/numbers/u32_12312312"));
	printf("value = %lu\n", mcfg_data_as_u32(*field));

	return 0;
}
```