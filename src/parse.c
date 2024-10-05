/* parse.c ; marie config format internal parser implementation
 * implementation for MCFG/2
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#include "parse.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XMALLOC(s)                                                             \
  ({                                                                           \
    void *ret;                                                                 \
    ret = malloc(s);                                                           \
    if (ret == NULL) {                                                         \
      return MCFG_MALLOC_FAIL;                                                 \
    }                                                                          \
    ret;                                                                       \
  })

#define ERR_CHECK_RET(val)                                                     \
  do {                                                                         \
    if (val != MCFG_OK) {                                                      \
      return val;                                                              \
    }                                                                          \
  } while (0)

#define TOKEN_CHECKED_SET(cnode, str, val, tk)                                 \
  do {                                                                         \
    if (strncmp(str, val, sizeof(val) - 1) == 0) {                             \
      _set_node(&cnode, tk, NULL);                                             \
      ix += sizeof(val) - 1;                                                   \
      continue;                                                                \
    }                                                                          \
  } while (0)

mcfg_err_t _set_node(syntax_tree_t **node, token_t token, char *value) {
  (*node)->token = token;
  (*node)->value = value;

  syntax_tree_t *new_current = XMALLOC(sizeof(syntax_tree_t));

  new_current->token = TK_UNASSIGNED_TOKEN;
  new_current->value = NULL;
  new_current->prev = *node;

  (*node)->next = new_current;

  *node = new_current;

  return MCFG_OK;
}

void _extract_string(void) {
  /* TODO */
  /*
    char *next_quote_ptr = strchrnul(input_offs, '\'');

    /* find real closing quote (ignore instances of '') */
  /*
    while (next_quote_ptr[0] != '\0' &&
            (next_quote_ptr[0] == '\'' && next_quote_ptr[1] == '\'')) {
      next_quote_ptr = strchrnul(next_quote_ptr + 1, '\'');
      fprintf(stderr, "still searching, %c\n", next_quote_ptr[0]);
    }

    /* if never closed, set the remaining input as a string */
  /*
    if (*next_quote_ptr == '\0') {
      fprintf(stderr, "nullptr!\n");
      _set_node(&current_node, TK_STRING, strdup(input_offs + 1));
      ix += next_quote_ptr - input;
      break;
    }

    size_t value_size = next_quote_ptr - (input_offs + 1);
    char *value = XMALLOC(value_size);
    strncpy(value, input_offs + 1, value_size);

    _set_node(&current_node, TK_STRING, value);

    ix += next_quote_ptr - input + 1;
  */
}

/* NOTE: This lexing structure does not really produce a tree, it is more like
 *       a linked list of tokens encountered within the input.
 */
mcfg_err_t lex_input(char *input, syntax_tree_t *tree) {
  if (input == NULL || tree == NULL) {
    return MCFG_NULLPTR;
  }

  syntax_tree_t *current_node = tree;

  size_t ix = 0;
  char cur_char = input[0];

  /* This loop doesn't actually go over every character by itself, in case of
   * e.g. a comment it searches for the next linefeed and, if found, jumps to
   * one char after it.
   */
  while (cur_char != '\0') {
    char *input_offs = input + ix;

    switch (cur_char) {
    case ';': { /* comment */
      char *lf_ptr = strchr(input + ix, '\n');
      if (lf_ptr == NULL) {
        break;
      }

      ix += lf_ptr - input;
      break;
    }
    case ',':
      _set_node(&current_node, TK_COMMA, NULL);
      break;
    case '\'': /* possibly a string open/close quote */
      _set_node(&current_node, TK_QUOTE, NULL);

      _extract_string(); /* TODO */

      break;
    case 'i': /* possibly a signed integer */
      TOKEN_CHECKED_SET(current_node, input_offs, "i8", TK_I8);
      TOKEN_CHECKED_SET(current_node, input_offs, "i16", TK_I16);
      TOKEN_CHECKED_SET(current_node, input_offs, "i32", TK_I32);
      break;
    case 'u': /* possibly an unsigned integer */
      TOKEN_CHECKED_SET(current_node, input_offs, "u8", TK_U8);
      TOKEN_CHECKED_SET(current_node, input_offs, "u16", TK_U16);
      TOKEN_CHECKED_SET(current_node, input_offs, "u32", TK_U32);
      break;
    case 'l': /* possibly a list */
      TOKEN_CHECKED_SET(current_node, input_offs, "list", TK_LIST);
      break;
    case 's': /* possibly a string, section or sector */
      TOKEN_CHECKED_SET(current_node, input_offs, "str", TK_STR);
      TOKEN_CHECKED_SET(current_node, input_offs, "sector", TK_SECTOR);
      TOKEN_CHECKED_SET(current_node, input_offs, "section", TK_SECTION);
      break;
    case 'e': /* possibly an end */
      TOKEN_CHECKED_SET(current_node, input_offs, "end", TK_END);
      break;
    default:
      break; /* something else */
    }

    ix++;
    cur_char = input[ix];
  }

  return MCFG_OK;
}

mcfg_err_t parse_tree(syntax_tree_t tree, mcfg_file_t *destination_file) {
  return MCFG_OK;
}
