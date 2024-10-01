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

typedef enum token {
  TK_UNASSIGNED_TOKEN = -1,

  /* Structuring */

  TK_SECTOR,
  TK_SECTION,
  TK_END,

  /* Misc. */

  TK_QUOTE,
  TK_COMMA,
  TK_UNKNOWN,

  /* Data Types */

  TK_STR,
  TK_LIST,
  TK_BOOL,
  TK_I8,
  TK_U8,
  TK_I16,
  TK_U16,
  TK_I32,
  TK_U32,

  /* Data Literals */

  TK_NUMBER,
  TK_STRING,
} token_t;

/* not really a tree */
struct syntax_tree {
  /** @brief The token enum of this entry in the "tree" */
  token_t token;

  /** @brief Optionally a value for this entry in the "tree" */
  char *value;

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
