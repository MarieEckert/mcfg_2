#include <stdio.h>
#include <stdlib.h>

#include "mcfg.h"

#include "testing_shared.c"

#define TEST_DIR   "tests/"

#define TEST_STEPS 1

#ifndef TEST_FILE
#	define TEST_FILE TEST_DIR "embedding_test.mcfg"
#endif

int
main(void)
{
	TEST_INFO;

	char *filepath = TEST_FILE;

	BEGIN_STEP("parsing file \"" TEST_FILE "\"");

	mcfg_parse_result_t ret = mcfg_parse_from_file(filepath);
	if(ret.err != MCFG_OK) {
		STEP_FAIL;

		fprintf(stderr, STEP_LOG_PRIMER "mcfg parsing failed: %s (%d)\n",
				mcfg_err_string(ret.err), ret.err);
		fprintf(stderr, STEP_LOG_PRIMER "in file \"%s\" on line %zu\n",
				filepath, ret.err_linespan.starting_line);
		exit(current_step);
	}

	STEP_SUCCESS;

	mcfg_free_file(ret.value);
	return 0;
}
