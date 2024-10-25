# MCFG/2
MCFG/2 is a reworking of my original [mcfg library](https://github.com/FelixEcker/mcfg) and format.

## Building
MCFG/2'S recommended way of building is to use [mariebuild](https://github.com/FelixEcker/mariebuild),
if you do not have a version of mariebuild installed which is compatible with
the build file format you can use the provided `scripts/build.bash` script.

**NOTE:** Before you can build using either of the provided methods, you should
run the `scripts/setup.bash` script!

## Overview
### Basic Library Usage
*For a more detailed guide for using this library, see the files in `doc/usage/`*

```c
#include <stdio.h>
#include <stdlib.h>

#include "mcfg.h"
#include "mcfg_util.h"

int main(void) {
  char path[] = "tests/number_types.mcfg";

  mcfg_file_t file;
  mcfg_err_t ret = mcfg_parse_file(path, &file);

  if (ret != MCFG_OK) {
    fprintf(stderr, "failed to parse file: %s\n", mcfg_err_string(ret));
    return ret;
  }

  mcfg_field_t *field = mcfg_get_field_by_path(&file, mcfg_parse_path("/test/numbers/u32_12312312"));
  printf("value = %lu\n", mcfg_data_as_u32(*field));

  return 0;
}
```

### Structuring
MCFG/2 files are always structured into sectors and sections. Sectors sit at the
top level and contain sections, within sections fields can be declared. Sectors
and sections have to be terminated using the `end` keyword.

Example:
```mcfg
sector foo
  section bar
    str my_field 'foobar'
  end
end
```

### Datatypes
- Number Types
  - bool
  - u8, u16, u32
  - i8, i16, i32
- Strings (multiline too!)
- Lists of the previous datatypes (multiline too!)

### Field Embedding
MCFG/2 supports embedding fields within string fields using the following
syntax: `$(FIELD_NAME)`. Fields from within the same section can be embedded by
simply inserting their name. If they come from another section they require an
absolute path to them.

Example:
```mcfg
sector sec1
  section origin
    str some_field 'some value'
  end
end

sector sec2
  section dest
    str some_other_field 'origin: $(/sec1/origin/some_field)'
  end
end
```

Dynamically generated fields can be inserted by wrapping their name between
percentage-signs.

Example:
`str some_field 'dynamic field value: $(%some_dynamic_field%)'`

## Repository Structure

* `src/` – Implementation Sourcecode
* `include/` – Headers for public Library Interface
* `misc/` – Miscellaneous things
* `scripts/` – Different scripts
* `tests/` – Test / Example files
* `README.md`
* `LICENSE`
* `build.mb`
* `.gitignore`
