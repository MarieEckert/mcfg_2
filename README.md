# MCFG/2
MCFG/2 is a reworking of my original [mcfg library](https://github.com/FelixEcker/mcfg).

## Building
MCFG/2'S recommended way of building is to use [mariebuild](https://github.com/FelixEcker/mariebuild),
if you do not have a version of mariebuild installed which is compatible with
the build file format you can use the provided `build.bash` script.

## Overview
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
  - **NOTE:** Multiline stringlists are currently not implemented

### Field Embedding (WIP)
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
    str some_other_field 'origin: $(/sec1/origin/some_field)`
  end
end
```

Dynamically generated fields can be inserted by wrapping their name between
percentage-signs.

Example:
`str some_field 'dynamic field value: $(%some_dynamic_field%)`

## Example
```mcfg
; MCFG/2 example file for mariebuild
sector config
  section mariebuild
    ; Build in full everytime instead of incrementally
    str build_mode 'full'

    str bin 'test'

    list str targets 'all', 'clean', 'compile'
  end

  section meta
    u16 version_major 1
    u16 version_minor 12
    u16 version_patch 1312
  end

  section source
    str src_dir 'src/'
    list str files 'util.c', 'main.c'
  end
end

sector targets
  section all
    list str required_targets 'clean', 'compile'
  end

  section clean
    list str rules 'prepare'

    str cmd 'rm obj/*'
  end

  section compile
    list str rules 'prepare', 'compile'

    str cmd 'gcc -o $(/config/mariebuild/bin) $(/config/source/sources)
  end
end

sector rules
  section prepare
    str cmd '
#!/bin/bash
if [[ ! -d obj/ ]]; then
  mkdir obj/
fi
    '
  end

  section compile
    str exec_on '/config/source/sources/'
    str exec

    str cmd 'gcc -c -o obj/$(%input%).o $(/config/source/src_dir)$(%input).c'
  end
end
```
