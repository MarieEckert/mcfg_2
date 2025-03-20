int current_step = 1;

#define TEST_INFO                                                          \
	do {                                                                   \
		fprintf(stderr, "\t>> Using MCFG/2 version " MCFG_2_VERSION "\n"); \
	} while(0)

#define BEGIN_STEP(msg)                                              \
	do {                                                             \
		fprintf(stderr, "\t[%d/%d] " msg, current_step, TEST_STEPS); \
		current_step++;                                              \
	} while(0)

#define STEP_LOG_PRIMER "\t    -> "

#define STEP_SUCCESS                                     \
	do {                                                 \
		fprintf(stderr, "  \x1b[32;1mSUCCESS\x1b[0m\n"); \
	} while(0)

#define STEP_FAIL                                       \
	do {                                                \
		fprintf(stderr, "  \x1b[31;1mFAILED\x1b[0m\n"); \
	} while(0)