# Lists
## Summary
Lists in MCFG/2 are declared using the `list` keyword followed by the lists type
and its name. Lists cant be made up of other lists.

Elements of a List are seperated using a comma, they may extend over multiple
lines. If a list is intended to extend over multiple lines, the comma before the
first element on the new line has to be placed before the line break.

Commas after the final element of a list are not permitted.

### Example
```mcfg
sector list_tests
  section str_lists
    list str str_list_single_line 'test', 'test aasd test'
  end
  section num_lists
    list u8 u8_list 255, 127, 64, 32, 16, 8, 4, 2, 1, 0
    list i8 i8_list_multiline -127, -64, -32, -16, -8, -4, -2, -1,
                              0,
                              1, 2, 4, 8, 16, 64, 127
    list u32 u32_list 1000000, 100000, 10000, 1000, 100, 10, 1, 0
    list bool bool_list true, true, false, 
    false, true, false, true,
    false, true, false, true, false,   ,   
    true
  end
end
```

## Internals
lists are represented using the type `mcfg_list_t`, they internally hold the type
of the list and its member fields.

### Sample of parsed list
```c
struct mcfg_list list_field {
  enum mcfg_field_type type = 4 (TYPE_U8)
  size_t field_count = 5
  struct mcfg_field *fields = [
    0 = 0xff
    1 = 0xe8
    // ...
  ]
}

struct mcfg_file {
  struct mcfg_sector *sectors = [
    0 = {
      struct mcfg_section *sections = [
        0 = {
          struct mcfg_field *fields = [
            0 = {
              char *name = test_list
              enum mcfg_field_type type = 1 (TYPE_LIST)
              void *data = (void*) &list_field
            }
          ]
        }
      ]
    }
  ]
}
```
