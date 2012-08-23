#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "builtin.h"
#include "pgrp.h"


struct builtin builtins_list[] = {
    {"cd",      builtin_cd},
    {"exit",    builtin_exit},
    {"fg",    builtin_fg},
    {"bg",    builtin_bg},
    {"jobs",    builtin_jobs},
    {NULL,      NULL}
};

struct builtin*
lookup_builtin(char *cmd)
{
    struct builtin *p;
	
    for (p = builtins_list; p->name; p++) {
        if (strcmp(cmd, p->name) == 0)
            return p;
    }
    return NULL;
}

int
builtin_cd(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "%s: wrong argument\n", argv[0]);
        return 1;
    }
    if (chdir(argv[1]) < 0) {
        perror(argv[1]);
        return 1;
    }
    return 0;
}

int
builtin_exit(int argc, char *argv[])
{
    if (argc != 1) {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return 1;
    }
    printf("\nByebye.\n\n");
    exit(0);
}

int
builtin_fg(int argc, char *argv[])
{
	int pgrp;
	if (argc !=2) {
		fprintf(stderr, "%s: wrong argument\n", argv[0]);
		return 1;
	}
	pgrp = atoi(argv[1]);
	if (search_pgrp(head, pgrp) == NULL) {
        fprintf(stderr, "%s: not found such a process group.\n", argv[0]);
        return 1;
    }
	tcsetpgrp(1, pgrp);
    //SIGCONTをおくる
    killpg(pgrp, SIGCONT);
    return 0;
}

int
builtin_bg(int argc, char *argv[])
{
	int pgrp;
	if (argc !=2) {
		fprintf(stderr, "%s: wrong argument\n", argv[0]);
		return 1;
	}
	pgrp = atoi(argv[1]);
	if (search_pgrp(head, pgrp) == NULL) {
        fprintf(stderr, "%s: not found such a process group.\n", argv[0]);
        return 1;
    }
    //SIGCONTをおくる
    killpg(pgrp, SIGCONT);
    return 0;
}

int
builtin_jobs(int argc, char *argv[])
{
	if (argc != 1) {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return 1;
    }
    show_pgrp(head);
    return 0;
}

