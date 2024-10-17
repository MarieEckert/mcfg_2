/* parse.c ; marie config format internal parser implementation
 * implementation for MCFG/2
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 2

#include "parse.h"

#include "shared.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAMESPACE parse

#define _set_node NAMESPACED_DECL(_set_node)
#define _process_mcfg_string NAMESPACED_DECL(_process_mcfg_string)
#define _extract_string NAMESPACED_DECL(_extract_string)
#define _extract_word NAMESPACED_DECL(_extract_word)

char *mcfg_token_str(token_t tk) {
  switch (tk) {
  case TK_UNASSIGNED_TOKEN:
    return "TK_UNASSIGNED_TOKEN";
  case TK_SECTOR:
    return "TK_SECTOR";
  case TK_SECTION:
    return "TK_SECTION";
  case TK_END:
    return "TK_END";
  case TK_QUOTE:
    return "TK_QUOTE";
  case TK_COMMA:
    return "TK_COMMA";
  case TK_UNKNOWN:
    return "TK_UNKNOWN";
  case TK_STR:
    return "TK_STR";
  case TK_LIST:
    return "TK_LIST";
  case TK_BOOL:
    return "TK_BOOL";
  case TK_I8:
    return "TK_I8";
  case TK_U8:
    return "TK_U8";
  case TK_I16:
    return "TK_I16";
  case TK_U16:
    return "TK_U16";
  case TK_I32:
    return "TK_I32";
  case TK_U32:
    return "TK_U32";
  case TK_NUMBER:
    return "TK_NUMBER";
  case TK_BOOLEAN:
    return "TK_BOOLEAN";
  case TK_STRING:
    return "TK_STRING";
  }
}

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
    const mcfg_err_t __err_check_ret_err = val;                                \
    if (__err_check_ret_err != MCFG_OK) {                                      \
      return __err_check_ret_err;                                              \
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
    ERR_CHECK_RET(_set_node(&cnode, tk, NULL, 1));                             \
    ix += sizeof(val) - 2;                                                     \
    break;                                                                     \
  }                                                                            \
  do {                                                                         \
  } while (0)

/**
 * @brief Essentially does the same as TOKEN_CHECKED_SET but also allows for the
 * value in the string to be followed by a comma. This will also set the value
 * field of the current node to be a copy of val.
 * @see TOKEN_CHECKED_SET
 */
#define LITERAL_TOKEN_CHECKED_SET(cnode, str, val, tk)                         \
  if (strncmp(str, val, sizeof(val) - 1) == 0 &&                               \
      (isspace(str[sizeof(val) - 1]) || str[sizeof(val) - 1] == '\0' ||        \
       str[sizeof(val) - 1] == ',')) {                                         \
    ERR_CHECK_RET(_set_node(&cnode, tk, strdup(val), 1));                      \
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
 * @param line_count The count of lines on which the node resides
 * @return MCFG_OK on success.
 */
mcfg_err_t _set_node(syntax_tree_t **node, token_t token, char *value,
                     size_t line_count) {
  (*node)->token = token;
  (*node)->value = value;
  (*node)->linespan.line_count = line_count;

  if ((*node)->linespan.starting_line == 0) {
    (*node)->linespan.starting_line =
        (*node)->prev != NULL ? (*node)->prev->linespan.starting_line : 1;
  }

  syntax_tree_t *new_current = XMALLOC(sizeof(syntax_tree_t));

  new_current->token = TK_UNASSIGNED_TOKEN;
  new_current->value = NULL;
  new_current->prev = *node;
  new_current->linespan.starting_line = 0;
  new_current->linespan.line_count = 1;

  (*node)->next = new_current;

  *node = new_current;

  return MCFG_OK;
}

/**
 * @brief result structure for the return value of _process_mcfg_string
 * @see _process_mcfg_string
 */
typedef struct _process_mcfg_string_res {
  /** @brief The error/status return value, MCFG_OK on success */
  mcfg_err_t err;

  /** @brief The resulting processed string, maybe NULL when err != MCFG_OK */
  char *result;

  /** @brief The amount of linefeeds encountered in the input */
  size_t linefeed_count;
} _process_mcfg_string_res_t;

/**
 * @brief Performs extra processing required for MCFG/2 strings. This includes:
 *    1. Making all double single-quotes ('') into single single-quotes (')
 * @param in The string value to process
 * @return A _process_mcfg_string_res_t struct
 * @see _process_mcfg_string_res
 */
_process_mcfg_string_res_t _process_mcfg_string(char *in) {
  _process_mcfg_string_res_t result = {
      .err = MCFG_TODO,
      .result = NULL,
      .linefeed_count = 0,
  };

  char *dest_buffer = malloc(strlen(in) + 1);
  if (dest_buffer == NULL) {
    result.err = MCFG_MALLOC_FAIL;
    return result;
  }

  char *next_quote_ptr = strchrnul(in, '\'');

  size_t copy_offset = 0;
  size_t write_offset = 0;

  /* copy everything up to and including the next single-quote,
   * then offset the "in" pointer to be 2 characters after the
   * single-quote to skip the second single-quote
   */
  while (next_quote_ptr[0] != '\0') {
    const size_t copy_amount = next_quote_ptr - in + 1;
    memcpy(dest_buffer + write_offset, in, copy_amount);

    write_offset += copy_amount;
    in += copy_amount + 1;
    next_quote_ptr = strchrnul(in, '\'');
  }

  memcpy(dest_buffer + write_offset, in + copy_offset, next_quote_ptr - in + 1);
  write_offset += next_quote_ptr - in + 1;

  /* count linefeeds in string so that they number of lines for nodes are
   * correct after a multiline string
   */
  char *next_linefeed_ptr = strchr(dest_buffer, '\n');
  while (next_linefeed_ptr != NULL) {
    result.linefeed_count++;

    next_linefeed_ptr = strchr(next_linefeed_ptr + 1, '\n');
  }

  /* This is something which is not really necessary but works better
   * in some rare cases.
   */
#ifdef _MCFG_REALLOC_LEXED_STRINGS
  dest_buffer = realloc(dest_buffer, write_offset);
  if (dest_buffer == NULL) {
    result.err = MCFG_MALLOC_FAIL;
    return result;
  }
#endif

  result.err = MCFG_OK;
  result.result = dest_buffer;
  return result;
}

/**
 * @brief extract a string from the input and set is as the current node.
 * @param node Pointer to the current node-pointer
 * @param input Pointer to the actual first character of the input
 * @param ix Pointer to the used index to be updated once done
 * @param line_number Pointer to the line number tracker so that in case of
 * multiline strings, extract string can properly increment it
 * @return MCFG_OK on success
 */
mcfg_err_t _extract_string(syntax_tree_t **node, char *input, size_t *ix,
                           size_t *line_number) {
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

  const size_t value_size = quote_ptr - input_offs + 1;
  char *value = XMALLOC(value_size);
  strncpy(value, input_offs, value_size - 1);
  value[value_size - 1] = '\0';

  /* do extra processing as specified in NOTE(1) */
  _process_mcfg_string_res_t processing_result = _process_mcfg_string(value);
  free(value);

  ERR_CHECK_RET(processing_result.err);

  if (processing_result.result == NULL) {
    return MCFG_NULLPTR;
  }

  *line_number += processing_result.linefeed_count;

  ERR_CHECK_RET(_set_node(node, TK_STRING, processing_result.result,
                          processing_result.linefeed_count));
  ERR_CHECK_RET(_set_node(node, TK_QUOTE, NULL, 1));

  *ix += value_size;

  return MCFG_OK;
}

/**
 * @brief extract a word from the input and set is as the current node.
 * @param node Pointer to the current node-pointer
 * @param input Pointer to the actual first character of the input
 * @param ix Pointer to the used index to be updated once done
 * @param tk The token enum value to be set for the current node
 * @return MCFG_OK on success
 */
mcfg_err_t _extract_word(syntax_tree_t **node, char *input, size_t *ix,
                         token_t tk) {
  /* Search where the current word ends */
  size_t search_ix = *ix + 1;

  while (input[search_ix] != '\0' && !isspace(input[search_ix])) {
    search_ix++;
  }

  /* Copy the word into a new buffer */
  const size_t value_size = search_ix - *ix + 1;
  char *value = XMALLOC(value_size);
  strncpy(value, input + *ix, value_size - 1);
  value[value_size - 1] = '\0';

  /* finish up */
  ERR_CHECK_RET(_set_node(node, tk, value, 1));

  /* subtract one from value_size because of null terminator */
  *ix += value_size - 2;
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

  size_t line_number = 1;

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
      _set_node(&current_node, TK_COMMA, NULL, 1);
      break;
    case '\'': /* possibly a string open/close quote */
      _set_node(&current_node, TK_QUOTE, NULL, 1);

      ERR_CHECK_RET(_extract_string(&current_node, input, &ix, &line_number));
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
    case 't': /* maybe a true literal */
      LITERAL_TOKEN_CHECKED_SET(current_node, input_offs, "true", TK_BOOLEAN);
      goto _default_case;
    case 'f': /* maybe a false literal */
      LITERAL_TOKEN_CHECKED_SET(current_node, input_offs, "false", TK_BOOLEAN);
      goto _default_case;
    case '-': /* maybe a negative number literal */
      if (!isdigit(input[ix + 1])) {
        goto _default_case;
      }

      ERR_CHECK_RET(_extract_word(&current_node, input, &ix, TK_NUMBER));
      break;
    case '\n':
      line_number++;
      current_node->linespan.starting_line = line_number;
      break;
    _default_case:
    default:
      /* ignore any whitespace outside of a string */
      if (isspace(cur_char)) {
        break;
      }

      /* add word as a number literal */
      if (isdigit(cur_char)) {
        ERR_CHECK_RET(_extract_word(&current_node, input, &ix, TK_NUMBER));
        break;
      }

      /* thrown in the word as an unknown token */
      ERR_CHECK_RET(_extract_word(&current_node, input, &ix, TK_UNKNOWN));
      break;
    }

    ix++;
    cur_char = input[ix];
  }

  return MCFG_OK;
}

/**
 * @brief Checks if the given error value is equal to MCFG_OK. If not, it will
 * cause the calling function to return with the `err` field being set to the
 * given error. The `err_linespan` field will be equal to `c->linespan`.
 * @param e The error value to check.
 * @param c Pointer to the current node.
 */
#define PARSER_ERR_CHECK_RET(e, c)                                             \
  do {                                                                         \
    mcfg_err_t _err = e;                                                       \
    if (_err != MCFG_OK) {                                                     \
      _parse_result_t _res;                                                    \
      _res.err = _err;                                                         \
      _res.err_linespan = c->linespan;                                         \
      return _res;                                                             \
    }                                                                          \
  } while (0)

/**
 * @brief Validates that the current state of the parsing function matches the
 * expected state. If it does not, it will cause the function to return with the
 * given error.
 * @param cstate The current parser state.
 * @param estate The expected parser state.
 * @param _err The error value to be set on mismatch.
 */
#define VALIDATE_PARSER_STATE(cstate, estate, _err)                            \
  if (cstate != estate) {                                                      \
    result.err = _err;                                                         \
    result.err_linespan = current->linespan;                                   \
    return result;                                                             \
  }                                                                            \
  do {                                                                         \
  } while (0)

/**
 * @brief used by the parse_tree function to keep track of its own state.
 */
typedef enum _parse_tree_state {
  /** @brief The parser is not inside of a section or sector */
  PTS_IDLE = 0,
  PTS_IN_SECTOR = 1,
  PTS_IN_SECTION = 2,
} _parse_tree_state_t;

_parse_result_t parse_tree(syntax_tree_t tree, mcfg_file_t *destination_file) {
  /* MCFG/2 Syntax Rules:
   *    0. Unless explicitly stated, a token can not appear by itself.
   *    1. The TK_SECTOR and TK_SECTION tokens are exclusively used to declare
   *       new sectors and sections respectively. They must always be followed
   *       by a TK_UNKNOWN token of which the value holds the name of the new
   *       sector/section.
   *    2. Any instance of a TK_STRING token should be prefixed by a TK_QUOTE
   *       token. A terminating TK_QUOTE token coming immediatly after a String
   *       closes it. If said TK_QUOTE token is missing, the string will be
   *       presumed to be unclosed and reult in an error.
   *    3. All datatype tokens (such as TK_STR, TK_U8, TK_BOOL, etc.) must be
   *       followed by a TK_UNKNOWN token of which the value hodls the name of
   *       the newly declared field. This token must then intern be followed by
   *       a data literal valid for the datatype of the field.
   *    4. The TK_LIST token must be followed by:
   *          1. A datatype token
   *          2. A TK_UNKNOWN token of which the value holds the name for the
   *             list
   *          3. A literal token, this can optionally be followed by a TK_COMMA
   *             and another literal token.
   */

  /* MCFG/2 Structural Rules:
   *    1. Sectors may only be declared at the top level,
   *    2. Sections may only be declared inside sectors.
   *    3. Fields may only be declared inside sections.
   *    4. Sectors and Sections must both be termianted before a new one can be
   *       opened using the TK_END token.
   *    5. A TK_END token outside of a Sector is invalid.
   */

  _parse_result_t result = {
      .err = MCFG_OK,
      .err_linespan = tree.linespan,
  };

  if (destination_file == NULL) {
    result.err = MCFG_NULLPTR;
    return result;
  }

  _parse_tree_state_t state = PTS_IDLE;

  syntax_tree_t *current = &tree;

  while (current != NULL) {
    switch (current->token) {
    case TK_UNASSIGNED_TOKEN:
      return result;
    case TK_SECTOR:
      VALIDATE_PARSER_STATE(state, PTS_IDLE, MCFG_STRUCTURE_ERROR);

      /* see syntax rule 1 at the start of the function */
      if (current->next == NULL || current->next->token != TK_UNKNOWN ||
          current->next->value == NULL) {
        result.err = MCFG_SYNTAX_ERROR;
        result.err_linespan = current->linespan;
        return result;
      }

      PARSER_ERR_CHECK_RET(
          mcfg_add_sector(destination_file, current->next->value), current);
      current = current->next;
      state = PTS_IN_SECTOR;
      break;
    case TK_SECTION:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTOR, MCFG_STRUCTURE_ERROR);

      /* see syntax rule 1 at the start of the function */
      if (current->next == NULL || current->next->token != TK_UNKNOWN ||
          current->next->value == NULL) {
        result.err = MCFG_SYNTAX_ERROR;
        result.err_linespan = current->linespan;
        return result;
      }

      PARSER_ERR_CHECK_RET(
          mcfg_add_section(
              &destination_file->sectors[destination_file->sector_count - 1],
              current->next->value),
          current);
      current = current->next;
      state = PTS_IN_SECTION;
      break;
    case TK_END:
      switch (state) {
      case PTS_IN_SECTOR:
        state = PTS_IDLE;
        break;
      case PTS_IN_SECTION:
        state = PTS_IN_SECTOR;
        break;
      default:
        result.err = MCFG_END_IN_NOWHERE;
        result.err_linespan = current->linespan;
        return result;
      }
      break;
    case TK_QUOTE:
      break;
    case TK_COMMA:
      break;
    case TK_UNKNOWN:
      result.err = MCFG_SYNTAX_ERROR;
      result.err_linespan = current->linespan;
      return result;
    case TK_STR:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_LIST:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_BOOL:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_I8:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_U8:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_I16:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_U16:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_I32:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_U32:
      VALIDATE_PARSER_STATE(state, PTS_IN_SECTION, MCFG_STRUCTURE_ERROR);
      break;
    case TK_NUMBER:
      result.err = MCFG_SYNTAX_ERROR;
      result.err_linespan = current->linespan;
      return result;
    case TK_BOOLEAN:
      result.err = MCFG_SYNTAX_ERROR;
      result.err_linespan = current->linespan;
      return result;
    case TK_STRING:
      result.err = MCFG_SYNTAX_ERROR;
      result.err_linespan = current->linespan;
      return result;
    }

    current = current->next;
  }

  return result;
}
