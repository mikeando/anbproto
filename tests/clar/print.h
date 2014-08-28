
static void clar_print_init(int test_count, int suite_count, const char *suite_names)
{
	(void)test_count;
	printf("Loaded %d suites: %s\n", (int)suite_count, suite_names);
	printf("Started\n");
}

static void clar_print_shutdown(int test_count, int suite_count, int error_count)
{
	(void)test_count;
	(void)suite_count;
	(void)error_count;

	printf("\n\n");
	clar_report_errors();
}

static void clar_print_error(int num, const struct clar_error *error)
{
	printf("  %d) Failure:\n", num);

	printf("%s::%s [%s:%d]\n",
		error->suite,
		error->test,
		error->file,
		error->line_number);

	printf("  %s\n", error->error_msg);

	if (error->description != NULL)
		printf("  %s\n", error->description);

	printf("\n");
	fflush(stdout);
}

static int sub_test_index =0;

static void clar_print_ontest(const char *test_name, int test_number, int failed)
{
	++sub_test_index;
	printf("   [%03d] %s : %s\n", sub_test_index, test_name, (failed)?"FAILED":"OK");
	fflush(stdout);
}

static void clar_print_onsuite(const char *suite_name, int suite_index)
{
	sub_test_index=0;
	printf("[%02d] %s\n", suite_index, suite_name);
}

static void clar_print_onabort(const char *msg, ...)
{
	va_list argp;
	va_start(argp, msg);
	vfprintf(stderr, msg, argp);
	va_end(argp);
}
