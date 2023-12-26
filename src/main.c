#include <stdio.h>
#include <stdlib.h>

#include "mcfg.h"

int main() {
  char *filepath = "doc/example.mb";

  mcfg_file_t *file = malloc(sizeof(mcfg_file_t));
  mcfg_parser_ctxt_t *ctxt;
  mcfg_err_t ret = mcfg_parse_file_ctxto(filepath, file, &ctxt);
  if (ret != MCFG_OK) {
    printf("mcfg parsing failed: %s (%d)\n", mcfg_err_string(ret), ret);
    printf("in file \"%s\" on line %d\n", filepath, ctxt->linenum);
    goto cleanup;
  }

cleanup:
  free(file);
  return 0;
}
