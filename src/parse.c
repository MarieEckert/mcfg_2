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

/**
 * @brief Set the current nodes token & value and append a new one if the given
 * string matches the first sizeof(str) - 1 characters of str and
 * str[sizeof(str) - 1] is either whitespace or NULL.
 * @param cnode Pointer to the current-node pointer
 * @param str The string to check in
 * @param val The value to check for
 * @param tk The token enum value to set on match
 */
#define TOKEN_CHECKED_SET(cnode, str, val, tk)                                 \
  if (strncmp(str, val, sizeof(val) - 1) == 0 &&                               \
      (isspace(str[sizeof(val) - 1]) || str[sizeof(val) - 1] == '\0')) {       \
    ERR_CHECK_RET(_set_node(&cnode, tk, NULL));                                \
    ix += sizeof(val) - 2;                                                     \
    break;                                                                     \
  }                                                                            \
  do {                                                                         \
  } while (0)

/**
 * @brief Set the token and the value of the current node and append a new one
 * onto it. This will cause the passed current node-pointer to be updated.
 * @param node A pointer to the current node-pointer.
 * @param token The token enum value to be set.
 * @param value The string value to be set, can be NULL. Ownership should be
 * considered to be transfered to the current node.
 * @return MCFG_OK on success.
 */
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

/**
 * @brief extract a string from the input and set is as the current node.
 * @param node Pointer to the current node-pointer
 * @param input Pointer to the actual first character of the input
 * @param ix Pointer to the used index to be updated once done
 * @return MCFG_OK on success
 */
mcfg_err_t _extract_string(syntax_tree_t **node, char *input, size_t *ix) {
  /* This function has two specific edge cases to handle when searching for the
   * closing quote:
   *    1. The quote may never be closed. In this case everything up to the NULL
   *       byte of the input should be included in the string.
   *    2. Any quote which is encountered may serve to "escape" a following
   *       quote. So if "''" is encountered, it does not close the string.
   *
   * NOTE(1):
   * Adding on to 2.: In case a "''" is encountered, one of the quotes must
   * eventually be removed from the input as this mechanism serves the same
   * functionality as double-quoting in pascal: Being able to insert a
   * single-quote within the single-quoted strings of this format.
   */
  const char *input_offs = input + *ix + 1; /* add one to avoid opening quote */
  char *quote_ptr = strchrnul(input_offs, '\'');

  while (quote_ptr[0] != '\0' &&
         (quote_ptr[0] == '\'' && quote_ptr[1] == '\'')) {
    size_t offset = 1;

    /* increase offset to avoid the "escaped" quote if there was one */
    if (quote_ptr[1] == '\'') {
      offset++;
    }

    quote_ptr = strchrnul(quote_ptr + offset, '\'');
  }

  /* TODO: do required extra steps as specified in NOTE(1) (or should this be
   * moved to the parsing step?)
   */
  const size_t value_size = quote_ptr - input_offs + 1;
  char *value = XMALLOC(value_size);
  strncpy(value, input_offs, value_size - 1);
  value[value_size - 1] = '\0';

  ERR_CHECK_RET(_set_node(node, TK_STRING, value));

  *ix += value_size + 1;

  return MCFG_OK;
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

      ERR_CHECK_RET(_extract_string(&current_node, input, &ix)); /* TODO */

      break;
    case 'i': /* possibly a signed integer */
      TOKEN_CHECKED_SET(current_node, input_offs, "i8", TK_I8);
      TOKEN_CHECKED_SET(current_node, input_offs, "i16", TK_I16);
      TOKEN_CHECKED_SET(current_node, input_offs, "i32", TK_I32);
      goto _default_case;
    case 'u': /* possibly an unsigned integer */
      TOKEN_CHECKED_SET(current_node, input_offs, "u8", TK_U8);
      TOKEN_CHECKED_SET(current_node, input_offs, "u16", TK_U16);
      TOKEN_CHECKED_SET(current_node, input_offs, "u32", TK_U32);
      goto _default_case;
    case 'l': /* possibly a list */
      TOKEN_CHECKED_SET(current_node, input_offs, "list", TK_LIST);
      goto _default_case;
    case 's': /* possibly a string, section or sector */
      TOKEN_CHECKED_SET(current_node, input_offs, "str", TK_STR);
      TOKEN_CHECKED_SET(current_node, input_offs, "sector", TK_SECTOR);
      TOKEN_CHECKED_SET(current_node, input_offs, "section", TK_SECTION);
      goto _default_case;
    case 'e': /* possibly an end */
      TOKEN_CHECKED_SET(current_node, input_offs, "end", TK_END);
      goto _default_case;
    _default_case:
    default:
      /* ignore any whitespace outside of a string */
      if (isspace(cur_char)) {
        break;
      }

      /* Search where the current word ends */
      size_t search_ix = ix + 1;

      while (input[search_ix] != '\0' && !isspace(input[search_ix])) {
        search_ix++;
      }

      /* Copy the word into a new buffer */
      const size_t value_size = search_ix - ix + 1;
      char *value = XMALLOC(value_size);
      strncpy(value, input + ix, value_size - 1);
      value[value_size - 1] = '\0';

      /* finish up */
      _set_node(&current_node, TK_UNKNOWN, value);

      /* subtract one from value_size because of null terminator */
      ix += value_size - 1;

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
