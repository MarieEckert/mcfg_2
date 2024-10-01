/* parse.c ; marie config format internal parser implementation
 * implementation for MCFG/2
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#include "parse.h"

#include <string.h>

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
    switch (cur_char) {
    case ';': { /* comment */
      char *lf_ptr = strchr(input + ix, '\n');
      if (lf_ptr == NULL) {
        break;
      }

      ix += lf_ptr - input;
      break;
    }
    case '\'': /* possibly a string open/close quote */
      break;
    case 'i': /* possibly a signed integer */
      break;
    case 'u': /* possibly an unsigned integer */
      break;
    case 'l': /* possibly a list */
      break;
    case 's': /* possibly a string, section or sector */
      break;
    case 'e': /* possibly an end */
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
