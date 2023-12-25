#include <stdio.h>
#include <stdlib.h>

#include "mcfg.h"

int main() {
  mcfg_file_t *file = malloc(sizeof(mcfg_file_t));
  mcfg_err_t ret = mcfg_parse_file("doc/example.mb", file);
  if (ret != MCFG_OK) {
    printf("mcfg parsing failed: %s (%d)\n", mcfg_err_string(ret), ret);
    goto cleanup;
  }

cleanup:
  free(file);
  return 0;
}
