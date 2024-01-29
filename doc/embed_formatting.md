# Embed Formatting
## Summary & Example
MCFG/2 supports embedding fields within string fields using the following
syntax: `$(FIELD_NAME)`. Fields from within the same section can be embedded by
simply inserting their name. If they come from another section they require an
absolute path to them.

### Example
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

## Dynamic Fields
Dynamically generated fields can be inserted by wrapping their name between
percentage-signs.

### Example:
`str some_field 'dynamic field value: $(%some_dynamic_field%)`

## List Field Embedding
If list fields are embedded as a whole, each element is inserted with a single
space to seperate them by default. Characters which come immeditaly before or
after the embed up until a space is encountered are pre- and postfixed to each
element.

### Example
```mcfg
list str some_list 'one', 'two', 'three'
str other 'fields: $(some_list),'
```

After formatting other, the result will be:
`fields: one, two, three,`

## List Element Embedding
If a single element from a list should be embedded, it can be accesed by
postfixing a colon to the name and then the 0-based index of the element.
If the index is out of range, nothing will be inserted instead.

### Example
```mcfg
list str some_list 'one', 'two', 'three'
str other 'field 2: $(some_list:1)'
str foo 'field 4: $(some_list:3)'
```

After formatting other, the result will be:
```
field 2: two
field 4:
```
