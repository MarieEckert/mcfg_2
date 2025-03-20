#include <stdio.h>
#include <stdlib.h>

#include "mcfg.h"

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

int
main(void)
{
	fprintf(stderr, "Using MCFG/2 version " MCFG_2_VERSION "\n");

	char *filepath = TEST_DIR "embedding_test.mcfg";

	fprintf(stderr, "\t* parsing initial\n");

	mcfg_parse_result_t ret = mcfg_parse(initial);
	if(ret.err != MCFG_OK) {
		fprintf(stderr, "mcfg parsing failed: %s (%d)\n",
				mcfg_err_string(ret.err), ret.err);
		fprintf(stderr, "in file \"%s\" on line %zu\n", filepath,
				ret.err_linespan.starting_line);
		exit(1);
	}

	fprintf(stderr, "\t* parsed initial\n\t* serializing\n");

	mcfg_serialize_result_t serialized_result =
		mcfg_serialize(ret.value, MCFG_DEFAULT_SERIALIZE_OPTIONS);
	if(serialized_result.err != MCFG_OK) {
		fprintf(stderr, "mcfg serialization failed: %s (%d)\n",
				mcfg_err_string(ret.err), ret.err);
		exit(2);
	}

	printf("%s\n", serialized_result.value->data);

	fprintf(stderr, "\t* serialized\n\t* parsing serialized\n");

	ret = mcfg_parse(serialized_result.value->data);
	if(ret.err != MCFG_OK) {
		fprintf(stderr, "mcfg parsing failed: %s (%d)\n",
				mcfg_err_string(ret.err), ret.err);
		fprintf(stderr, "in file \"%s\" on line %zu\n", filepath,
				ret.err_linespan.starting_line);
		exit(1);
	}

	fprintf(stderr, "\t* parsed serialized\n");

	mcfg_free_file(ret.value);
	return 0;
}
