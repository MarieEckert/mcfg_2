#include <stdlib.h>

#include <stdio.h>

#include "mcfg.h"

#define TEST_DIR "tests/"

int
main(void)
{
	fprintf(stderr, "Using MCFG/2 version " MCFG_2_VERSION "\n");

	char *filepath = TEST_DIR "embedding_test.mcfg";

	mcfg_parse_result_t ret = mcfg_parse_from_file(filepath);
	if(ret.err != MCFG_OK) {
		fprintf(stderr, "mcfg parsing failed: %s (%d)\n",
				mcfg_err_string(ret.err), ret.err);
		fprintf(stderr, "in file \"%s\" on line %zu\n", filepath,
				ret.err_linespan.starting_line);
		exit(1);
	}

	fprintf(stderr, "parsed you a mcfg file!\n");
	mcfg_free_file(ret.value);
	return 0;
}
