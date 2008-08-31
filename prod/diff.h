/*
 *  diff.h
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

#ifndef _DIFF_H
#define _DIFF_H

#define DIFF_MAX_LINE_LENGTH (256 * 1024)

enum {
    D_NONE = 0,
    D_ADD,
    D_DEL,
    D_EDIT
};

/**
 * A changed or edited line on the right side of the diff
 */
typedef struct {
    char *line;
} rd_line_t;

typedef struct {
    int ignore_case;
    int ignore_all_whitespace;
    int ignore_whitespace_changes;
    int ignore_blank_lines;
} rd_diff_opts_t;

/**
 * A single difference
 */
typedef struct {
    int f1_line_start;
    int f1_n_lines;
    int f2_line_start;
    int f2_n_lines;
    
    int d_type;
    
    rd_line_t *changes;      ///< This is an array of add or edits to the right file (f2).
    int n_changes;           ///< Number of changes in use in the changes list
    int n_changes_alloc;     ///< Number of changes allocated in the changes list

    int *deletes;            ///< This is a list of line numbers (that index into the original file)
                             ///< that represent lines we are deleting from the original file. 
    int n_deletes;           ///< Number of deletes in use in the deletes list 
    int n_deletes_alloc;     ///< Number of deletes allocated in the deletes list

    int effective_line;      ///< The effective line number that this diff is at
} rd_diff_t;

/**
 * Original file representation
 */
typedef int my_size_t;
typedef struct {
    unsigned long magic;
    
    char *bytes;
    my_size_t n_bytes;
    int *newlines;
    my_size_t n_newlines;

    int n_effective_lines;    ///< Number of effective lines in the summed output.
    
    rd_diff_t *diffs;
    my_size_t n_diffs;
    my_size_t n_diffs_alloc;
    
    rd_diff_opts_t opts;    ///< Options that were used to make this diff
    char f1_name[512];
    char f2_name[512];
    
    int longest_left_line;
    int longest_right_line;
} rd_file_t;

/**
 * XXX: Need well defined return codes
 *
 * @param f1 (i) Name of first file
 * @param f2 (i) name of second file
 * @param out_file (o) Pointer to an allocated rd_file_t
 */
void
diff_file(const char *f1, const char *f2, rd_file_t *out_file, rd_diff_opts_t *opts);

/**
 * Return TRUE if diffing looks like it will work (i.e., /usr/bin/diff exists)
 */
int
diff_is_supported(void);

/**
 * XXX: Operates on the global g_file
 */
void
diff_free_file(void);

/**
 * Given a diff type, return a printable string.
 *
 * @param dt (i) Diff type value.
 * @return String name for \dt.
 */
const char *
diff_type_to_string(int dt);

/**
 * XXX:
 */
rd_diff_t *
diff_get_diff_for_line(rd_file_t *f, int lineno);

rd_diff_t *
diff_get_next_diff_for_effective_line(rd_file_t *f, int eff_line);

rd_diff_t *
diff_get_prev_diff_for_effective_line(rd_file_t *f, int eff_line);


int
diff_file_loaded(rd_file_t *f);

const rd_diff_opts_t *
diff_get_opts(rd_file_t *f);

void
diff_set_opts(rd_file_t *f, rd_diff_opts_t *opts);

void
diff_load_opts_from_prefs(rd_diff_opts_t *opts);

/**
 * Returns the length (number of columns) of the longest line in the original file
 */
int
diff_get_longest_line_on_left(rd_file_t *f);

int
diff_get_longest_line_on_right(rd_file_t *f);

int
diff_get_length_for_orig_line(rd_file_t *f, int lineno);

// XXX: HACK
extern rd_file_t g_file;

#endif
