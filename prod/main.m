/*
 *  main.m
 *  roaringdiff - http://www.biscade.com/tools/diff/

/* 
 * Copyright © 2006-2008 Mitch Haile.
 *
 * This file is part of RoaringDiff.
 *
 * RoaringDiff is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * RoaringDiff is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <Cocoa/Cocoa.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "diff.h"
#include "rd_prefs.h"

typedef struct {
	int use_files;
	char f1[PATH_MAX];
	char f2[PATH_MAX];
	
	char label_1[PATH_MAX];	/* GNU diff supports -L arguments to label the files */
	char label_2[PATH_MAX];	/* GNU diff supports -L arguments to label the files */
	
	int use_prefs;
} cmd_opts_t;

static 	cmd_opts_t g_opts;

int get_cmd_files(const char **f1, const char **f2, const char **left_label, const char **right_label);

static void
init_cmd_opts(cmd_opts_t *o)
{
	assert(o != NULL);
	bzero(o, sizeof(*o));
	
	o->use_prefs = 1;
}

static void
usage(void)
{
	fprintf(stderr, "Usage: rdiff [-u] [-L <left-label> -L <right-label>] <left-file> <right-file>\n");
	fprintf(stderr, "Note that -u is ignored, but supported for default svn diff behavior.\n");
	exit(1);
}

static void
parse_cmd_opts(int argc, char **argv, cmd_opts_t *o)
{
	assert(argv != NULL);
	assert(o    != NULL);
	
	o->use_files = 0;
	
	/* Right now we don't support any options except for the two file names :-) */
	if (argc == 1) {
		/* Nothing */
		return;
	} else if (argc == 2) {
		/* see if the 2nd arg is the -psn_* "process serial number" argument from the Finder */
		if (strncmp(argv[1], "-psn_", 5) == 0) {
			/* OK. */
		} else {
			usage();
		}
	} else {
		/*
		 * svn calls diff normally with:
		 * -u -L <label_1> -L <label_2> <file_1> <file_2>
		 */
		int i = 0;
		while (i < argc - 2) {
			if (strcmp(argv[i], "-L") == 0) {
				i++;
				if (o->label_1[0] == '\0') {
					strncpy(o->label_1, argv[i], sizeof(o->label_1));
				} else if (o->label_2[0] == '\0') {
					strncpy(o->label_2, argv[i], sizeof(o->label_2));
				} else {
					usage();
				}
			}
			
			i++;
		}
		
		/* Two files */
		char *rp_f1 = realpath(argv[argc - 2], o->f1);
		char *rp_f2 = realpath(argv[argc - 1], o->f2);
		if (rp_f1 == NULL) {
			strncpy(o->f1, argv[argc - 2], sizeof(o->f1));
		}
		
		if (rp_f2 == NULL) {
			strncpy(o->f2, argv[argc - 1], sizeof(o->f2));
		}
		
		o->f1[sizeof(o->f1) - 1] = '\0';
		o->f2[sizeof(o->f2) - 1] = '\0';

		/*
		 * If the labels have not been set, then copy the file names to the labels
		 */
		if (o->label_1[0] == '\0') {
			strncpy(o->label_1, o->f1, sizeof(o->label_1));
		}
		
		if (o->label_2[0] == '\0') {
			strncpy(o->label_2, o->f2, sizeof(o->label_2));
		}
		
		fprintf(stderr,
				"RoaringDiff comparing...\n"
				"   %s (%s)\n"
				"   %s (%s)\n",
				o->label_1, o->f1, o->label_2, o->f2);

		o->use_files = 1;
	}
}

static void
validate_cmd_opts(cmd_opts_t *o)
{
	assert(o != NULL);
	
	if (o->use_files) {
		struct stat sbuf;
		if (stat(o->f1, &sbuf) != 0) {
			fprintf(stderr, "...Cannot stat %s: %s (will treat as empty)\n", o->f1, strerror(errno));
		}

		if (stat(o->f2, &sbuf) != 0) {
			fprintf(stderr, "...Cannot stat %s: %s (will treat as empty)\n", o->f2, strerror(errno));
		}
	}
}

int
get_cmd_files(const char **f1, const char **f2, const char **left_label, const char **right_label)
{
	*f1 = NULL;
	*f2 = NULL;
	*left_label = NULL;
	*right_label = NULL;

	if (g_opts.use_files) {
		*f1 = g_opts.f1;
		*f2 = g_opts.f2;
		
		if (g_opts.label_1[0] != '\0') {
			*left_label = g_opts.label_1;
		}
		
		if (g_opts.label_2[0] != '\0') {
			*right_label = g_opts.label_2;
		}
		return 1;
	}
	
	return 0;
}

static void
dump_opts(int argc, char **argv)
{
#if 1
	(void) argc;
	(void) argv;
#else
	int i;
	for (i = 0; i < argc; i++) {
		fprintf(stderr, "argv[%d] = __%s__\n", i, argv[i]);
	}
	
	system("env");
#endif
}

int
main(int argc, char *argv[])
{
	dump_opts(argc, argv);
	init_cmd_opts(&g_opts);
	parse_cmd_opts(argc, argv, &g_opts);
	validate_cmd_opts(&g_opts);
	
	if (g_opts.use_prefs) {
		/* 
		 * XXX:  This is not complete - we still need to tell the prefs not to write anything back if we're not using
		 * XXX:  the prefs
		 */
		rd_prefs_init();
	}
	
    return NSApplicationMain(argc,  (const char **) argv);
}
