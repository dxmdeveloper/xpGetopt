#include "xpgetopt.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef unsigned int uint;
#define true 1
#define false 0

char *xpoptarg = NULL;
int xpoptind = 1; /* argv argument index */
int xpopterr = 1; /* print message about unrecognized option to stderr if not set to 0 */
int xpoptopt = '?';

static struct xpoption * find_long_option(struct xpoption long_opt_arr[], const char * name){
    for(;long_opt_arr->name; long_opt_arr++)
        if(strcmp(long_opt_arr->name, name) == 0) return long_opt_arr;
    return NULL;
}

static int find_long_option_index(struct xpoption long_opt_arr[], const char * name){
    int i;
    size_t namelen = 0;
    for(i = 0; long_opt_arr[i].name; i++) {
        /* optimization - in most cases the line below will skip not matching option without checking whole name */
        if(long_opt_arr[i].name[0] != name[0]) continue;

        namelen = strlen(long_opt_arr[i].name);
        if (strncmp(long_opt_arr[i].name, name, namelen) == 0
        && (!name[namelen] || name[namelen] == '=')) return i;
    }
    return -1;
}

/** @brief Function checks if given argument is option or option argument.
  * Indexes of argv arguments after -- terminator must not be passed.
  * @param lopts may be NULL, but then option argument of long option will return false */
static int is_opt_or_argopt_check(char* argv[], uint argvind, char * options, struct xpoption * lopts) {
	char *optchloc = NULL;
    uint i;
	/* is option check */
	if (argv[argvind][0] == '-' && argv[argvind][1]) return true;

	/* if previous is not an option then this is not an argoptg */
	if (argvind <= 1 || argv[argvind-1][0] != '-' || !argv[argvind-1][1]) return false;

	/* argument of long option. */
	if (argv[argvind - 1][1] == '-'){
        struct xpoption *longopt = NULL;
        if(!lopts) return false;
        /* below will work correctly even if long option has `=` sign */
        if((longopt = find_long_option(lopts, argv[argvind-1]+2))){
            return longopt->has_arg == required_argument ? true : false;
        }
    }

	/* check if it's argopt of short opt */
	for (i = 1; isalnum(argv[argvind - 1][i]); i++) {
		optchloc = strchr(options, argv[argvind - 1][i]);
		if (optchloc && optchloc[1] == ':') {
			if (!argv[argvind - 1][i + 1]) return true;
			else return false;
		}
	}

	return false;
}

/** @brief Function that reorders arg vector arguments. non-option arguments are relocated to the end of the vector.
  * @return first non-option argument index */
static uint argv_reorder(int argc, char *argv[], char *options, void * long_options) {
    uint optend = 0;
    uint reloc_cnt = 0;
    uint i;

    for (i = argc - 1; i >= 1; i--) {
        if (strcmp(argv[i], "--") == 0
            || ((!optend || options[0] == '+' || options[1] == '+') && is_opt_or_argopt_check(argv, i, options, long_options)))
            optend = i;
    }

    for (i = optend - 1; i >= 1; i--) {
        if (!is_opt_or_argopt_check(argv, i, options, long_options)) {
            uint ii;
            char *arg2mov = argv[i];
            for (ii = i; ii < optend - reloc_cnt; ii++) {
                argv[ii] = argv[ii + 1];
            }
            argv[optend - reloc_cnt] = arg2mov;
            reloc_cnt++;
        }
    }
    return optend - reloc_cnt + 1;
}

static int parse_short(char *argv[], char *options, char **nextchpp, uint non_opt_start){
    char retval = '\0';
    char *optstrcharloc = NULL;

    optstrcharloc = strchr(options, **nextchpp);
    if (!optstrcharloc || !isalnum(**nextchpp)) {
        /* unrecognized option */
        xpoptopt = **nextchpp;
        if (xpopterr) fprintf(stderr, "unrecognized option: %c\r\n", **nextchpp);
        retval = '?';
    }
    else if (optstrcharloc[1] == ':') {
        /* option with argument */
        if ((*nextchpp)[1]) {
            xpoptarg = *nextchpp + 1;
            retval = **nextchpp;
        }
        else if (xpoptind + 1 < non_opt_start) {
            xpoptarg = argv[xpoptind++ + 1];
            retval = **nextchpp;
        }
        else {
            if (xpopterr && options[0] != ':') fprintf(stderr, "missing option argument\r\n");
            retval = options[0] == ':' ? ':' : '?';
        }
    }
    else retval = **nextchpp;

    xpoptind++;
    if(*nextchpp && !*(*nextchpp)++) *nextchpp = NULL;
    return retval;
}

static int parse_long(char *argv[], char * options, void *long_options, int *opt_indexp, uint non_opt_start){
    char * eq_sign = NULL;
    int loption_index = 0;
    struct xpoption * loption = NULL;

    eq_sign = strchr(argv[xpoptind], '=');

    loption_index = find_long_option_index(long_options, argv[xpoptind]+2);
    xpoptind++;
    if(opt_indexp) *opt_indexp = loption_index;
    if(loption_index == -1){
        if (xpopterr) fprintf(stderr, "unrecognized option: %s\r\n", argv[xpoptind-1]+2);
        return '?';
    }
    loption = &((struct xpoption *)long_options)[loption_index];
    /* option argument parse */
    if(loption->has_arg == required_argument){
        if(eq_sign) {
            xpoptarg = eq_sign +1;
        }
        else if(xpoptind + 1 < non_opt_start) xpoptarg = argv[xpoptind++];
        else {
            if(xpopterr && options[0] != ':') fprintf(stderr, "missing option argument\r\n");
            if(loption->flag) *loption->flag = options[0] == ':' ? ':' : '?';
            else xpoptopt = loption->value;
            return options[0] == ':' ? ':' : '?';
        }
    }

    if(loption->flag) {
        *loption->flag = loption->value;
        return 0;
    }
    return loption->value;
}

static int getopt_body(int argc, char *argv[], char *options, void *long_options, int *opt_indexp){
    static char *nextch = NULL;
    static uint non_opt_start;

    if (xpoptind == 0) xpoptind = 1;
    if (xpoptind == 1) non_opt_start = argv_reorder(argc, argv, options, long_options);
    if (xpoptind >= non_opt_start) {
        if(xpoptind < argc && strcmp(argv[xpoptind], "--") == 0) xpoptind++;
        return -1;
    }

    for (; xpoptind < non_opt_start; xpoptind++) {

        /* long option */
        if (argv[xpoptind][1] == '-'){
            if(!long_options) continue;
            return parse_long(argv, options, long_options, opt_indexp, non_opt_start);
        }

        /* short option */
        if (!nextch) nextch = &argv[xpoptind][1];
        return parse_short(argv, options, &nextch, non_opt_start);
    }
    return -1; /* the line should never be executed */
}

int xpgetopt(int argc, char *argv[], char *options){
	return getopt_body(argc, argv, options, NULL, NULL);
}

int xpgetopt_long(int argc, char **argv, char *options, void *long_options, int *opt_indexp) {
    return getopt_body(argc, argv, options, long_options, opt_indexp);
}
