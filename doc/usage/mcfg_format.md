# mcfg_format usage
## Introduction
The mcfg_format part of MCFG/2 is used to format so-called "field embeds", which
is a way to embed the contents of another field inside of a string-type field.

### Field Embed Example
```
sector foo
    section bar1
        str example 'value of /foo/bar2/test is: ''$(bar2/test)'''
    end

    section bar2
        str test 'lorum ipsum'
    end
end
```

### Usage Example
Formatting the value of the field `/foo/bar1/example` should yield the result `value of /foo/bar2/test is: 'lorum ipsum'`,
this can be accomplished using the following code.

```c
#include <stdio.h>

#include "mcfg.h"
#include "mcfg_format.h"

int main(void) {
  mcfg_file_t file;
  /* ... load file ... */

  char raw_field_path[] = "/foo/bar1/example";
  mcfg_path_t field_path = mcfg_parse_path(raw_field_path);

  mcfg_field_t *field = mcfg_get_field_by_path(&file, field_path);

  /* field_path is used for the path-relativity here, so that the fields relative
   * embeds work properly
   */
  mcfg_fmt_res_t fmt_res = mcfg_format_field_embeds(*field, file, field_path);

  if (fmt_res.err != MCFG_FMT_OK) {
    return 1;
  }

  printf("formatted:\n%s\n", fmt_res.formatted);
  return 0;
}
```