#ifndef _XPGETOPT_H_
#define _XPGETOPT_H_

#define no_argument 0
#define required_argument 1
/* optional arguments are not implemented */

/* 
* Author: Maciej Bąbolewski (dxmdeveloper)
* License: MIT
* Description: Portable implementation of POSIX getopt made to parse program arguments.
* Optional arguments are not implemented. Does not check for POSIXLY_CORRECT env var.
* https://man7.org/linux/man-pages/man3/getopt.3.html
*/

/* global vars */
extern char *xpoptarg;
extern int xpoptind; /* argv argument index                                                 */
extern int xpopterr; /* print message about unrecognized option to stderr if not set to 0   */
extern int xpoptopt; /* unrecognized option character                                       */

/* option struct */
struct xpoption {
    char *name;
    int has_arg;
    int *flag;
    int value;
};

/* functions */
int xpgetopt(int argc, char *argv[], char *options);

int xpgetopt_long(int argc, char *argv[], char *options, void *long_options, int *opt_indexp);

#endif