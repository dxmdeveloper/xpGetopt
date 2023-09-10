#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "xpgetopt.h"

typedef unsigned int uint;


int main(int argc, char ** argv){
	char opt = -1;
    char *test_argv[] = {
        "test.exe",
		"-p",
		"kasztaniaki",
		"--",
		"-t",
        "okno"
    };
	const int test_argc = sizeof(test_argv) / sizeof(char*);

	while ((opt = xpgetopt(test_argc, test_argv, "fp:")) != -1) {
		switch (opt) {
			case 'f':
				puts("ogórek\n");  break;
			case 'p':
				puts(xpoptarg); break;
		}
	}

    return 0;

}
