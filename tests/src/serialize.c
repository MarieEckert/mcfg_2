#include <stdio.h>
#include <stdlib.h>

#include "mcfg.h"

#include "testing_shared.c"

#define TEST_DIR "tests/"

char *initial =
	"sector embedding_test\n"
	"  section sect1\n"
	"    str foo 'bar'\n"
	"    u16 someu16 1337\n"
	"\n"
	"    str local_embed_test '$(foo)'\n"
	"  end\n"
	"end\n"
	"sector list_src\n"
	"  section sect1\n"
	"    list str somelist 'aasd', 'bebis', "
	"'$(/embedding_test/sect1/someu16)'\n"
	"  end\n"
	"end\n"
	"sector test\n"
	"  section sectneg1\n"
	"    i16 test -200\n"
	"    str a 'üüäü'\n"
	"  end\n"
	"\n"
	"  section sect1\n"
	"    bool booltest true\n"
	"    bool b2bool2test false\n"
	"\n"
	"    list bool boolllist true, false, false, true,\n"
	"    true\n"
	"\n"
	"    u8 test 200\n"
	"    str stest 'asdasd $(sectneg1/a)'\n"
	"    str dest 'foo $(/embedding_test/sect1/foo) with someu16\n"
	"    $(booltest) $(b2bool2test) $(boolllist)\n"
	"    $(/embedding_test/sect1/someu16) this should appear normal -> \\$(aa) "
	"''\n"
	"    abc__<$(/list_src/sect1/somelist)>@@ a\n"
	"    $(/bogus/field) $(test) $(sectneg1/test)\n"
	"    $(stest)\n"
	"  '\n"
	"  end\n"
	"end\n";

#define TEST_STEPS 3

mcfg_file_t
test_parse_original()
{
	BEGIN_STEP("parsing original");

	mcfg_parse_result_t ret = mcfg_parse(initial);
	if(ret.err != MCFG_OK) {
		STEP_FAIL;

		fprintf(stderr, STEP_LOG_PRIMER "mcfg parsing failed: %s (%d)\n",
				mcfg_err_string(ret.err), ret.err);
		fprintf(stderr, STEP_LOG_PRIMER "on line %zu\n",
				ret.err_linespan.starting_line);
		exit(current_step);
	}

	STEP_SUCCESS;

	return ret.value;
}

mcfg_string_t *
test_serialize(mcfg_file_t file)
{
	BEGIN_STEP("serializing");

	mcfg_serialize_result_t serialized_result =
		mcfg_serialize(file, MCFG_DEFAULT_SERIALIZE_OPTIONS);
	if(serialized_result.err != MCFG_OK) {
		STEP_FAIL;

		fprintf(stderr, STEP_LOG_PRIMER "mcfg serialization failed: %s (%d)\n",
				mcfg_err_string(serialized_result.err), serialized_result.err);
		exit(current_step);
	}

	if(serialized_result.value == NULL) {
		STEP_FAIL;

		fprintf(stderr, STEP_LOG_PRIMER
				"mcfg_serialize returned MCFG_OK but the value is NULL!\n");
		exit(current_step);
	}

	printf("%s\n", serialized_result.value->data);

	STEP_SUCCESS;
	return serialized_result.value;
}

mcfg_file_t
test_parse_serialized(mcfg_string_t *serialized)
{
	BEGIN_STEP("parsing serialized");

	mcfg_parse_result_t ret = mcfg_parse(serialized->data);
	if(ret.err != MCFG_OK) {
		STEP_FAIL;

		fprintf(stderr, STEP_LOG_PRIMER "mcfg parsing failed: %s (%d)\n",
				mcfg_err_string(ret.err), ret.err);
		fprintf(stderr, STEP_LOG_PRIMER "on line %zu\n",
				ret.err_linespan.starting_line);
		exit(current_step);
	}

	STEP_SUCCESS;

	return ret.value;
}

int
main(void)
{
	TEST_INFO;

	mcfg_file_t parsed = test_parse_original();
	mcfg_string_t *serialized = test_serialize(parsed);
	parsed = test_parse_serialized(serialized);

	return 0;
}
