#include <stdio.h>
#include <string.h>
#include "xpgetopt.h"

int xpgetopt_main(int argc, char ** argv){
	int opt = -1;
    int flag = 0;

    struct xpoption loptions[] = {
        {"test", required_argument, NULL, 't'},
        {"flag", no_argument, &flag, 1},
        {NULL, 0, NULL, 0}
    };

    char *test_argv[] = {
        "test.exe",
        "--test=argument",
        "--test",
        "argument again",
        "-t",
        "arg of short opt",
        "-Tinline_arg",
        "--flag",
        "-p",
        "--",
        "end of options",
        "-k"
    };
	const int test_argc = sizeof(test_argv) / sizeof(test_argv[0]);

	while ((opt = xpgetopt_long(test_argc, test_argv, "t:pkT:", loptions, NULL)) != -1) {
		switch (opt) {
            case 'T':
			case 't':
                printf("%c option. Option argument: %s\n",opt, xpoptarg); break;
			case 'p':
            case 'k':
				printf("%c option\n", opt); break;
		}
	}
    printf("flag value: %d\n", flag);

    return 0;

}
