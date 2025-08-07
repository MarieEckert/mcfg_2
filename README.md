# MCFG/2
MCFG/2 is a reworking of my original [mcfg library](https://github.com/FelixEcker/mcfg) and format.

## Building
MCFG/2'S recommended way of building is to use [mariebuild](https://git.heroin.trade/marie/mariebuild),
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

## Contributing
The coding style should be rather self-explanatory, be sure to run
clang-format on the files you have modified before committing.

Also see [the git styleguide](./GIT_STYLEGUIDE.md) and be sure to adhere to
the message format and the branching/history rules.

### Repository Structure

* `src/` – Implementation Sourcecode
* `include/` – Headers for public Library Interface
* `misc/` – Miscellaneous things
* `scripts/` – Different scripts
* `tests/` – Test / Example files
* `README.md`
* `LICENSE`
* `build.mb`
* `.gitignore`

### Links

* [radicle mirror](https://app.radicle.xyz/nodes/seed.radicle.garden/rad:z46Fd9SK3Lajw9b4u1DMvejCz7mb6)
