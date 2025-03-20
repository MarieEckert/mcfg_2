#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mcfg.h"
#include "mcfg_util.h"

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

#define TEST_STEPS 4

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

void
test_compare_structs(mcfg_file_t a, mcfg_file_t b)
{
	BEGIN_STEP("comparing original and newly parsed");

	if(a.sector_count != b.sector_count) {
		STEP_FAIL;

		fprintf(stderr, STEP_LOG_PRIMER "sector count mismatch");
		exit(current_step);
	}

	for(size_t ix = 0; ix < a.sector_count; ix++) {
		mcfg_sector_t a_sector = a.sectors[ix];
		mcfg_sector_t b_sector = b.sectors[ix];

		if(strcmp(a_sector.name, b_sector.name) != 0) {
			STEP_FAIL;

			fprintf(stderr, STEP_LOG_PRIMER "sector order / name mismatch!\n");
			fprintf(stderr, STEP_LOG_PRIMER "\"%s\" vs \"%s\"\n", a_sector.name,
					b_sector.name);
			exit(current_step);
		}

		if(a_sector.section_count != b_sector.section_count) {
			STEP_FAIL;

			fprintf(stderr,
					STEP_LOG_PRIMER "section count mismatch in sector \"%s\"",
					a_sector.name);
			exit(current_step);
		}

		for(size_t jx = 0; ix < a_sector.section_count; ix++) {
			mcfg_section_t a_section = a_sector.sections[jx];
			mcfg_section_t b_section = b_sector.sections[jx];

			if(strcmp(a_section.name, b_section.name) != 0) {
				STEP_FAIL;

				fprintf(stderr,
						STEP_LOG_PRIMER "section order / name mismatch!\n");
				fprintf(stderr, STEP_LOG_PRIMER "\"%s\" vs \"%s\"\n",
						a_section.name, b_section.name);
				exit(current_step);
			}

			if(a_section.field_count != b_section.field_count) {
				STEP_FAIL;

				fprintf(stderr,
						STEP_LOG_PRIMER
						"field_count count mismatch in sectiob \"%s\"",
						a_section.name);
				exit(current_step);
			}

			for(size_t kx = 0; ix < a_sector.section_count; ix++) {
				mcfg_field_t a_field = a_section.fields[kx];
				mcfg_field_t b_field = b_section.fields[kx];

				if(strcmp(a_field.name, b_field.name) != 0) {
					STEP_FAIL;

					fprintf(stderr,
							STEP_LOG_PRIMER "field order / name mismatch!\n");
					fprintf(stderr, STEP_LOG_PRIMER "\"%s\" vs \"%s\"\n",
							a_field.name, b_field.name);
					exit(current_step);
				}

				if(a_field.type != b_field.type) {
					STEP_FAIL;

					fprintf(stderr,
							STEP_LOG_PRIMER "field order / type mismatch!");
					fprintf(stderr, STEP_LOG_PRIMER "%d vs %d\n", a_field.type,
							b_field.type);
					exit(current_step);
				}

				switch(a_field.type) {
					case TYPE_STRING:
					case TYPE_LIST:
						if(strcmp(mcfg_data_as_string(a_field),
								  mcfg_data_as_string(b_field)) != 0) {
							STEP_FAIL;

							fprintf(stderr,
									"field value mismatch for field \"%s\"\n",
									a_field.name);
							exit(current_step);
						}
						break;
					case TYPE_BOOL:
						if(mcfg_data_as_bool(a_field) !=
						   mcfg_data_as_bool(b_field)) {
							STEP_FAIL;

							fprintf(stderr,
									"field value mismatch for field \"%s\"\n",
									a_field.name);
							exit(current_step);
						}
						break;
					case TYPE_I8:
					case TYPE_U8:
					case TYPE_I16:
					case TYPE_U16:
					case TYPE_I32:
					case TYPE_U32:
						if(mcfg_data_as_int(a_field) !=
						   mcfg_data_as_int(b_field)) {
							STEP_FAIL;

							fprintf(stderr,
									"field value mismatch for field \"%s\"\n",
									a_field.name);
							exit(current_step);
						}
						break;
					case TYPE_INVALID:
						STEP_FAIL;

						fprintf(stderr,
								STEP_LOG_PRIMER
								"invalid type encountered! field \"%s\"\n",
								a_field.name);
						exit(current_step);
				}
			}
		}
	}

	STEP_SUCCESS;
}

int
main(void)
{
	TEST_INFO;

	mcfg_file_t parsed = test_parse_original();
	mcfg_string_t *serialized = test_serialize(parsed);
	mcfg_file_t new_parsed = test_parse_serialized(serialized);
	test_compare_structs(parsed, new_parsed);

	return 0;
}
