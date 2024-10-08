/* parse.h ; marie config format internal parser header
 * for MCFG/2
 *
 * Copyright (c) 2024, Marie Eckert
 * Licensend under the BSD 3-Clause License.
 */

#ifndef PARSE_H
#define PARSE_H

#include "mcfg.h"

#include <stdbool.h>

typedef struct syntax_tree syntax_tree_t;

/**
 * @param Enum for every possible token within the MCFG/2 format.
 */
typedef enum token {
  /** @brief Helper value for initialising variables */
  TK_UNASSIGNED_TOKEN = -1,

  /* Structuring */

  /** @brief Corresponds to the "sector" keyword */
  TK_SECTOR,

  /** @brief Corresponds to the "section" keyword */
  TK_SECTION,

  /** @brief Corresponds to the "end" keyword */
  TK_END,

  /* Misc. */

  /** @brief Corresponds to a single-quote character */
  TK_QUOTE,

  /** @brief Corresponds to a comma character */
  TK_COMMA,

  /** @brief Used for words/characters without a known meaning */
  TK_UNKNOWN,

  /* Data Types */

  /** @brief Corresponds to the "str" keyword */
  TK_STR,

  /** @brief Corresponds to the "list" keyword */
  TK_LIST,

  /** @brief Correspodns to the "bool" keyword */
  TK_BOOL,

  /** @brief Correspodns to the "i8" keyword */
  TK_I8,

  /** @brief Correspodns to the "u8" keyword */
  TK_U8,

  /** @brief Correspodns to the "i16" keyword */
  TK_I16,

  /** @brief Correspodns to the "u16" keyword */
  TK_U16,

  /** @brief Correspodns to the "i32" keyword */
  TK_I32,

  /** @brief Correspodns to the "u32" keyword */
  TK_U32,

  /* Data Literals */

  /** @brief Used to represent number literals, including boolean values */
  TK_NUMBER,

  /** @brief Used to represent string literals */
  TK_STRING,
} token_t;

/**
 * @brief Get a string corresponding to the token identifier by the value.
 * @return The string.
 */
char *mcfg_token_str(token_t tk);

/**
 * @brief Structure to keep track on which lines a node / token sits.
 */
typedef struct linespan {
  /** @brief The first line on which it resides */
  size_t starting_line;

  /**
   * @brief The number of lines it resides. line_count == 1 should be taken to
   * mean that it only lives on one single line. 0 is the default value but can
   * also be interpreted to be a 1.
   */
  size_t line_count;
} linespan_t;

/**
 * @brief Struct to represent a lexed MCFG/2 file in a Double-Linked-List
 *        format.
 *
 * A syntax "tree" hsa to follow these rules:
 *    1. Any token which describes a keyword or single character must have value
 *       set to NULL.
 *    2. TK_UNKNOWN should be used for any word which does not match a keyword
 *       and is outside of a string. This includes sector/section/field names.
 *    3. A TK_STRING token must come after a TK_QUOTE token.
 *    4. A TK_QUOTE token after a TK_STRING token may only be set if the string
 *       is actually terminated by the end of the string and not by the EOF.
 *    5. Number literals must be stored with the TK_NUMBER token, this includes
 *       boolean values.
 *    6. The tree must be terminated with a token value of TK_UNASSIGNED_VALUE
 *       and the value set to NULL.
 */
struct syntax_tree {
  /** @brief The token enum of this entry in the "tree" */
  token_t token;

  /** @brief Optionally a value for this entry in the "tree" */
  char *value;

  /** @brief The span of lines the node in the "tree" takes up */
  linespan_t linespan;

  /** @brief The previous entry in the "tree" */
  syntax_tree_t *prev;

  /** @brief The next entry in the "tree" */
  syntax_tree_t *next;
};

/**
 * @brief Lexes the input string
 * @param input The entire input which is to be lexed
 * @param tree Pointer to write the result to
 * @return MCFG_OK on success
 */
mcfg_err_t lex_input(char *input, syntax_tree_t *tree);

/**
 * @brief Parses the given syntax tree into a mcfg_file_t struct
 * @param tree The tree to be parsed
 * @param mcfg Pointer to write the result to
 * @return MCFG_OK on success
 */
mcfg_err_t parse_tree(syntax_tree_t tree, mcfg_file_t *mcfg);

#endif // ifndef PARSE_H
