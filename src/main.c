#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

#include "mcfg.h"
#include "mcfg_format.h"
#include "mcfg_util.h"
#include "parse.h"

#define TEST_DIR "tests/"

void
print_file(mcfg_file_t file)
{
	printf("mcfg_file_t {\n");
	printf("  size_t dynfield_count = %zu\n", file.dynfield_count);
	printf("  mcfg_field_t *dynfields = [\n");
	for(size_t i = 0; i < file.dynfield_count; i++) {
		printf("    %zu = {\n", i);
		printf("      char *name = %s\n", file.dynfields[i].name);
		printf("      mcfg_field_type_t type = %d\n", file.dynfields[i].type);
		printf("      size_t size = %zu\n", file.dynfields[i].size);
		char *data_str = mcfg_data_to_string(file.dynfields[i]);
		printf("      void *data = %s\n", data_str);
		free(data_str);
		printf("    }\n");
	}
	printf("  ]\n");
	printf("  size_t sector_count = %zu\n", file.sector_count);
	printf("  mcfg_sector_t *sectors = [\n");
	for(size_t i = 0; i < file.sector_count; i++) {
		printf("    %zu = {\n", i);
		printf("      char *name = %s\n", file.sectors[i].name);
		printf("      size_t section_count = %zu\n",
			   file.sectors[i].section_count);
		printf("      mcfg_section_t *sections = [\n");
		for(size_t j = 0; j < file.sectors[i].section_count; j++) {
			printf("        %zu = {\n", j);
			printf("          char *name = %s\n",
				   file.sectors[i].sections[j].name);
			printf("          size_t field_count = %zu\n",
				   file.sectors[i].sections[j].field_count);
			printf("          mcfg_field_t *fields = [\n");
			for(size_t k = 0; k < file.sectors[i].sections[j].field_count;
				k++) {
				printf("            %zu = {\n", k);
				printf("              char *name = %s\n",
					   file.sectors[i].sections[j].fields[k].name);
				printf("              mcfg_field_type_t type = %d\n",
					   file.sectors[i].sections[j].fields[k].type);
				printf("              size_t size = %zu\n",
					   file.sectors[i].sections[j].fields[k].size);
				char *data_str =
					mcfg_data_to_string(file.sectors[i].sections[j].fields[k]);
				printf("              void *data = %s\n", data_str);
				free(data_str);
				printf("            }\n");
			}
			printf("          ]\n");
			printf("        }\n");
		}
		printf("      ]\n");
		printf("    }\n");
	}
	printf("  ]\n");
	printf("}\n");
}

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

	mcfg_file_t file = ret.value;
	print_file(file);

	char rel_path[] = "/test/sect1";
	mcfg_path_t rel = mcfg_parse_path(rel_path);
	mcfg_fmt_res_t res = mcfg_format_field_embeds(
		file.sectors[2].sections[1].fields[5], file, rel);
	if(res.err != MCFG_FMT_OK) {
		fprintf(stderr, "formatting error: %d\n", res.err);
	} else {
		fprintf(stderr, "format res =\n%s\n", res.formatted);
	}

	if(res.formatted != NULL)
		free(res.formatted);

	free(rel.sector);
	free(rel.section);
	free(rel.field);
	mcfg_free_file(file);
	return 0;
}
