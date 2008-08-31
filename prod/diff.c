/*
 *  diff.c
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
 * along with RoaringDiff.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

#include "diff.h"
#include "rd_prefs.h"

#define GNU_DIFF_PATH "/usr/bin/diff"
#define RD_FILE_MAGIC 0x19690622

rd_file_t g_file;
rd_file_t *the_file = &g_file;

static rd_diff_t *
diff_new(void)
{
    rd_diff_t *d = calloc(1, sizeof(rd_diff_t));
    assert(d != NULL);
    
    return d;
}

/**
 * Attach the diff object to the file
 * OK for diff object to be NULL.
 */
static void
diff_attach(rd_file_t *f, rd_diff_t *d)
{
    const int alloc_num = 32;

    assert(f != NULL);
    if (d == NULL) {
        return;
    }

    // If this is the first change, then we need to alloc the list for the first time
    if (f->diffs == NULL) {
        assert(f->n_diffs == 0);
        assert(f->n_diffs_alloc == 0);
        
        f->diffs = calloc(alloc_num, sizeof(rd_diff_t));
        assert(f->diffs != NULL);
        f->n_diffs_alloc = alloc_num;
    }
    
    // Are we out of room?
    if (f->n_diffs_alloc <= f->n_diffs + 1) {
        f->diffs = realloc(f->diffs, (alloc_num + f->n_diffs_alloc) * sizeof(rd_diff_t));
        assert(f->diffs != NULL);
        f->n_diffs_alloc += alloc_num;
        // Zero out the tail
        bzero(&f->diffs[f->n_diffs], alloc_num * sizeof(rd_diff_t));
    }
    
    // There had better be room!
    assert(f->n_diffs + 1 <= f->n_diffs_alloc);
    
    // Add it
    f->diffs[f->n_diffs] = *d;
    f->n_diffs++;
}

/**
 * Attach a line to a diff object.
 */
static void
line_attach(rd_diff_t *d, const char *s)
{
    const int alloc_num = 8;

    assert(d != NULL);
    assert(s != NULL);
    
    // If this is the first change, then we need to alloc the list for the first time
    if (d->changes == NULL) {
        assert(d->n_changes == 0);
        assert(d->n_changes_alloc == 0);
        
        d->changes = calloc(alloc_num, sizeof(rd_line_t));
        assert(d->changes != NULL);
        d->n_changes_alloc = alloc_num;
    }
    
    // Are we out of room?
    if (d->n_changes_alloc <= d->n_changes + 1) {
        d->changes = realloc(d->changes, (alloc_num + d->n_changes_alloc) * sizeof(rd_line_t));
        assert(d->changes != NULL);
        d->n_changes_alloc += alloc_num;
        // Zero out the tail
        bzero(&d->changes[d->n_changes], alloc_num * sizeof(rd_line_t));
    }
    
    // There had better be room!
    assert(d->n_changes + 1 <= d->n_changes_alloc);
    
    // Add it
    char *copy_s = strdup(s);
    assert(copy_s != NULL);
    d->changes[d->n_changes].line = copy_s;
    d->n_changes++;
}

/**
 * XXX: This implementation is pretty silly.
 */
static void
init_newlines(rd_file_t *f)
{
    assert(f != NULL);
    assert(f->bytes != NULL);
    assert(f->newlines == NULL);
    
    // Count the number of newlines */
    int num_newlines = 0;
    int i;
    for (i = 0; i < f->n_bytes; i++) {
        if (f->bytes[i] == '\n') {      // XXX: What about Mac files w/ \r or DOS with \r\n ?
            num_newlines++;
        }
    }

    if (num_newlines == 0) {
        // XXX: No newlines!
        return;
    }

    // Allocate some memory
    f->newlines = calloc(num_newlines + 1, sizeof(int));
    if (f->newlines == NULL) {
        return;  // XXX:
    }
    
    // The first line is at position 0
    int nl_index = 0;
    f->newlines[nl_index] = 0;
    nl_index++;
    
    // Scan for newlines */
    for (i = 0; i < f->n_bytes && nl_index <= num_newlines; i++) {
        if (f->bytes[i] == '\n') {     // XXX: What about Mac files w/ \r or DOS with \r\n ?
            // Without the check, this could be out of bounds on a file ending with a newline
            f->newlines[nl_index] = i + 1 > f->n_bytes ? i : i + 1;

            nl_index++;
            f->bytes[i] = '\0';                // Force the \0 terminator
        }
    }

    f->n_newlines = num_newlines;
}

/**
 * Load the specified file into an rd_file_t structure.
 *
 * This is the original file.
 * 
 * Returns 0 on success.
 */
static int
load_file(const char *fname, rd_file_t *f)
{
    assert(fname != NULL);
    assert(f != NULL);
    int fd = open(fname, O_RDONLY);
    if (fd < 0) {
        perror("open() failed");
        return -1;
    }
    
    off_t file_end = lseek(fd, 0, SEEK_END);
    if (file_end < 0) {
        perror("lseek() failed");
        close(fd);
        return -1;
    }
    
    // Nothing here
    if (file_end == 0) {
        close(fd);
        return 0;
    }
    
    // Allocate some memory (add 1 for the \0 terminator) */
    f->bytes = calloc(file_end + 1, sizeof(char));
    if (f->bytes == NULL) {
        perror("malloc() failed");
        return -1;
    }
    f->n_bytes = file_end + 1;

    // Reset the cursor.
    (void) lseek(fd, 0, 0);

    // Read in the bytes
    ssize_t bytes_read = read(fd, (void*) f->bytes, f->n_bytes);
    if (bytes_read < 0) {
        perror("read() failed");
        return -1;
    }
    
    if ((off_t) bytes_read != file_end) {
        fprintf(stderr, "%s: bytes_read(%lu) != file_end(%lld)\n", __FUNCTION__, bytes_read, file_end);
        return -1;
    }
    
    // Close the file
    close(fd);
    fd = -1;
    
    init_newlines(f);
    if (f->newlines == NULL) {
        return -1;
    }

    return 0;
}

static int g_pushback = 0;
static int g_gotone = 0;

static void
pushback_diff_line(void)
{
    // XXX: Right now we only support one line. 
    // Be sure we haven't already pushed back and that we've gotten a line already.
    assert(g_pushback == 0);
    assert(g_gotone != 0);
    g_pushback = 1;
}

static const char *
next_diff_line(FILE *p)
{
    assert(p != NULL);

    static char line[DIFF_MAX_LINE_LENGTH]; // XXX: could overflow
    if (g_pushback) {
        assert(g_gotone != 0);
        g_pushback = 0;
        return line;
    }

    bzero(line, sizeof(line));
    
    (void) fgets(line, sizeof(line), p);
    if (line[0] == '\0') {
        return NULL;
    }

    g_gotone = 1;
    
    // chop newline
    char *nl = strchr(line, '\n');
    if (nl != NULL) {
        *nl = '\0';
    }
    
    return line;
}

static void
handle_diff_set_type(rd_diff_t *d, char c)
{
    assert(d != NULL);
    d->d_type = D_NONE;
    switch (c) {
    case 'd':
        d->d_type = D_DEL;
        break;
    case 'a':
        d->d_type = D_ADD;
        break;
    case 'c':
        d->d_type = D_EDIT;
        break;
    }
}

static void
track_line_delete(rd_diff_t *d, int lineno)
{
    assert(d != NULL);
    
    const int alloc_num = 8;

    // If this is the first delete, then we need to alloc the list for the first time
    if (d->deletes == NULL) {
        assert(d->n_deletes == 0);
        assert(d->n_deletes_alloc == 0);
        
        d->deletes = calloc(alloc_num, sizeof(int));
        assert(d->deletes != NULL);
        d->n_deletes_alloc = alloc_num;
    }
    
    // Are we out of room? 
    if (d->n_deletes_alloc <= d->n_deletes + 1) {
        d->deletes = realloc(d->deletes, (alloc_num + d->n_deletes_alloc) * sizeof(int));
        assert(d->deletes != NULL);
        d->n_deletes_alloc += alloc_num;
        // Zero out the tail
        bzero(&d->deletes[d->n_deletes], alloc_num * sizeof(int));
    }
    
    // There had better be room!
    assert(d->n_deletes + 1 <= d->n_deletes_alloc);
    
    // Add it
    d->deletes[d->n_deletes] = lineno;
    d->n_deletes++;
}

/**
 *
 * The following is from 'info diff' describing the CHANGE-COMMAND format:
 *
 *
 * Detailed Description of Normal Format
 * -------------------------------------
 * 
 *    The normal output format consists of one or more hunks of
 * differences; each hunk shows one area where the files differ.  Normal
 * format hunks look like this:
 * 
 *      CHANGE-COMMAND
 *      < FROM-FILE-LINE
 *      < FROM-FILE-LINE...
 *      ---
 *      > TO-FILE-LINE
 *      > TO-FILE-LINE...
 * 
 *    There are three types of change commands.  Each consists of a line
 * number or comma-separated range of lines in the first file, a single
 * character indicating the kind of change to make, and a line number or
 * comma-separated range of lines in the second file.  All line numbers are
 * the original line numbers in each file.  The types of change commands
 * are:
 * 
 * `LaR'
 *      Add the lines in range R of the second file after line L of the
 *      first file.  For example, `8a12,15' means append lines 12-15 of
 *      file 2 after line 8 of file 1; or, if changing file 2 into file 1,
 *      delete lines 12-15 of file 2.
 * 
 * `FcT'
 *      Replace the lines in range F of the first file with lines in range
 *      T of the second file.  This is like a combined add and delete, but
 *      more compact.  For example, `5,7c8,10' means change lines 5-7 of
 *      file 1 to read as lines 8-10 of file 2; or, if changing file 2 into
 *      file 1, change lines 8-10 of file 2 to read as lines 5-7 of file 1.
 * 
 * `RdL'
 *      Delete the lines in range R from the first file; line L is where
 *      they would have appeared in the second file had they not been
 *      deleted.  For example, `5,7d3' means delete lines 5-7 of file 1;
 *      or, if changing file 2 into file 1, append lines 5-7 of file 1
 *      after line 3 of file 2.
 */
static void
handle_diff_change_command(const char *line, rd_file_t *f, rd_diff_t **d)
{
    assert(line != NULL);
    assert(f != NULL);
    assert(d != NULL);

    int left_start = 0,
        left_end = 0,
        right_start = 0,
        right_end = 0;
    char diff_type = 0;
    if (5 == sscanf(line,
                    "%d,%d%c%d,%d",
                    &left_start,
                    &left_end,
                    &diff_type,
                    &right_start,
                    &right_end)) {
        // Got a multi-line change [BOTH SIDES]
        diff_attach(f, *d);
        *d = diff_new();
        (**d).f1_line_start = left_start - 1;
        (**d).f1_n_lines    = left_end - left_start + 1;
        (**d).f2_line_start = right_start - 1;
        (**d).f2_n_lines    = right_end - right_start + 1;
        
        handle_diff_set_type(*d, diff_type);
    } else if (4 == sscanf(line, "%d%c%d,%d", &left_start, &diff_type, &right_start, &right_end)) {
        // Got a multi-line change [RIGHT]
        diff_attach(f, *d);
        *d = diff_new();
        
        (**d).f1_line_start = left_start - 1;
        (**d).f1_n_lines    = 1;
        (**d).f2_line_start = right_start - 1;
        (**d).f2_n_lines    = right_end - right_start + 1;

        // In this case, we're doing something on the right.
        // XXX: Always adding?
        handle_diff_set_type(*d, diff_type);
    } else if (4 == sscanf(line, "%d,%d%c%d", &left_start, &left_end, &diff_type, &right_start)) {
        // Got a multi-line change [LEFT]
        diff_attach(f, *d);
        *d = diff_new();
        
        (**d).f1_line_start = left_start - 1;
        (**d).f1_n_lines    = left_end - left_start + 1;
        (**d).f2_line_start = right_start - 1;
        (**d).f2_n_lines    = 1;

        handle_diff_set_type(*d, diff_type);
    } else if (3 == sscanf(line, "%d%c%d", &left_start, &diff_type, &right_start)) {
        // No... A single line change [BOTH SIDES]
        diff_attach(f, *d);
        *d = diff_new();
        
        (**d).f1_line_start = left_start - 1;
        (**d).f1_n_lines    = 1;
        (**d).f2_line_start = right_start - 1;
        (**d).f2_n_lines    = 1;

        handle_diff_set_type(*d, diff_type);
    } else {
        // No idea what this is
        fprintf(stderr, "%s: __%s__\n", __FUNCTION__, line);
    }

    /*
     * XXX: We don't currently handle the case where something is inserted
     * XXX: before the rest of the file!
     */
    assert((**d).f1_line_start >= -1);
}

/**
 * The following is from 'info diff' describing the CHANGE-COMMAND format:
 *
 * `RdL'
 *      Delete the lines in range R from the first file; line L is where
 *      they would have appeared in the second file had they not been
 *      deleted.  For example, `5,7d3' means delete lines 5-7 of file 1;
 *      or, if changing file 2 into file 1, append lines 5-7 of file 1
 *      after line 3 of file 2.
 */
static void
handle_diff_del(FILE *p, rd_diff_t *d)
{
    assert(p != NULL);
    assert(d != NULL);

    // consume lines until there's another digit line.
    int num_lines = 0;
    while (1) {
        const char *line = next_diff_line(p);
        if (line == NULL) {
            break;
        }
        
        if (isdigit(line[0])) {
            // pushback the last line
            pushback_diff_line();
            break;
        }
        
        assert(line[0] == '<');
        track_line_delete(d, d->f1_line_start + num_lines);
        num_lines++;
    }
}

/**
 * The following is from 'info diff' describing the CHANGE-COMMAND format:
 *
 * `LaR'
 *      Add the lines in range R of the second file after line L of the
 *      first file.  For example, `8a12,15' means append lines 12-15 of
 *      file 2 after line 8 of file 1; or, if changing file 2 into file 1,
 *      delete lines 12-15 of file 2.
 */
static void
handle_diff_add(FILE *p, rd_diff_t *d)
{
    assert(p != NULL);
    assert(d != NULL);

    while (1) {
        const char *line = next_diff_line(p);
        if (line == NULL) {
            break;
        }
        
        if (isdigit(line[0])) {
            // pushback the last line
            pushback_diff_line();
            break;
        }
        
        line_attach(d, line);
    }
}

/**
 * The following is from 'info diff' describing the CHANGE-COMMAND format:
 *
 * `FcT'
 *      Replace the lines in range F of the first file with lines in range
 *      T of the second file.  This is like a combined add and delete, but
 *      more compact.  For example, `5,7c8,10' means change lines 5-7 of
 *      file 1 to read as lines 8-10 of file 2; or, if changing file 2 into
 *      file 1, change lines 8-10 of file 2 to read as lines 5-7 of file 1.
 */
static void
handle_diff_edit(FILE *p, rd_diff_t *d)
{
    assert(p != NULL);
    assert(d != NULL);
    
    // consume lines until there's another digit line.
    int num_lines = 0;
    int got_separator = 0;
    while (1) {
        const char *line = next_diff_line(p);
        if (line == NULL) {
            break;
        }
        
        if (isdigit(line[0])) {
            // pushback the last line
            pushback_diff_line();
            break;
        }
        
        if (line[0] == '-') {
            assert(got_separator == 0);
            got_separator = 1;
        } else {
            if (got_separator) {
                line_attach(d, line);
            } else {
                assert(line[0] == '<');
                int lineno = d->f1_line_start + num_lines;
                track_line_delete(d, lineno);
                num_lines++;
            }
        }
    }
}

static void
build_diff_cmd(const char *f1, const char *f2, rd_diff_opts_t *opts, char *cmd, my_size_t sizeof_cmd)
{
    assert(f1   != NULL);
    assert(f2   != NULL);
    assert(opts != NULL);

    char opts_str[512];
    bzero(opts_str, sizeof(opts_str));
    
    if (opts->ignore_case) {
        strcat(opts_str, " --ignore-case");
    }
    
    if (opts->ignore_all_whitespace) {
        strcat(opts_str, " --ignore-all-space");
    }
    
    if (opts->ignore_whitespace_changes) {
        strcat(opts_str, " --ignore-space-change");
    }
    
    if (opts->ignore_blank_lines) {
        strcat(opts_str, " --ignore-blank-lines");
    }
    
    snprintf(cmd, sizeof_cmd, GNU_DIFF_PATH " -N %s %s %s", opts_str, f1, f2);
    cmd[sizeof_cmd - 1] = '\0';
}

static void
load_diff(const char *f1, const char *f2, rd_file_t *f, rd_diff_opts_t *opts)
{
    assert(f1 != NULL);
    assert(f2 != NULL);
    assert(f  != NULL);
    assert(opts     != NULL);
    assert(f->diffs == NULL);
    
    char cmd[1024];    // XXX: Could overflow
    build_diff_cmd(f1, f2, opts, cmd, sizeof(cmd));
    FILE *p = popen(cmd, "r");    // XXX: Obvious system() issues
    assert(p != NULL);
    
    rd_diff_t *d = NULL;
    while (1) {
        const char *line = next_diff_line(p);
        if (line == NULL) {
            break;
        }
        
        // see if the line is the beginning of a new change.
        if (isdigit(line[0])) {
            handle_diff_change_command(line, f, &d);
            assert(d != NULL);
            switch (d->d_type) {
            case D_DEL:
                handle_diff_del(p, d);
                break;
            case D_ADD:
                handle_diff_add(p, d);
                break;
            
            case D_EDIT:
                handle_diff_edit(p, d);
                break;
                
            default:
                fprintf(stderr, "%s: UNKNOWN DIFF TYPE!\n", __FUNCTION__);
                exit(1);
            }
    
            continue;
        } else {
            fprintf(stderr, "%s: UNEXPECTED TOKEN: %s\n", __FUNCTION__, line);
            break;
        }
    }
    
    // Attach any dangling diff laying around
    diff_attach(f, d);
    pclose(p);
    p = NULL;
}

static inline int MY_MAX(int a, int b)
{
    return a > b ? a : b;
}

static void
diff_calc_effective_lines(rd_file_t *f)
{
    assert(f != NULL);
    
    int effective_lines = 1;
    int left_line;
    for (left_line = -1; left_line < f->n_newlines; left_line++) {
        rd_diff_t *d = diff_get_diff_for_line(f, left_line);
        if (d != NULL) {
            switch (d->d_type) {
            case D_ADD:
                effective_lines++;    // Remember, we ADD *after* this line!
                d->effective_line = effective_lines;
                effective_lines += d->n_changes;
                break;
            case D_DEL:
                d->effective_line = effective_lines;
                effective_lines += d->n_deletes;
                break;
            case D_EDIT:
                d->effective_line = effective_lines;
                effective_lines += MY_MAX(d->n_deletes, d->n_changes);
                break;
            case D_NONE:
            default:
                break;
            }
            
            left_line += d->f1_n_lines - 1;
        } else {
            effective_lines++;
        }
    }
    
    f->n_effective_lines = effective_lines;
}

#pragma mark -
#pragma mark *** Public API ***

int
diff_is_supported(void)
{
    struct stat sbuf;
    if (stat(GNU_DIFF_PATH, &sbuf) != 0) {
        return 0;
    }

    return 1;
}

void
diff_file(const char *f1, const char *f2, rd_file_t *out_file, rd_diff_opts_t *opts)
{
    assert(f1 != NULL);
    assert(f2 != NULL);
    assert(out_file != NULL);
    assert(opts     != NULL);

    if (out_file == the_file && out_file->magic == RD_FILE_MAGIC) {
        diff_free_file();
    }
    
    bzero(out_file, sizeof(*out_file));
    out_file->magic = RD_FILE_MAGIC;
    
    strncpy(out_file->f1_name, f1, sizeof(out_file->f1_name));
    strncpy(out_file->f2_name, f2, sizeof(out_file->f2_name));
    out_file->f1_name[sizeof(out_file->f1_name) - 1] ='\0';
    out_file->f2_name[sizeof(out_file->f2_name) - 1] ='\0';
    
    load_file(f1, out_file);
    load_diff(f1, f2, out_file, opts);

    out_file->opts = *opts;

    diff_calc_effective_lines(out_file);
}

const char *
diff_type_to_string(int dt)
{
    switch (dt) {
    case D_NONE:    return "(none)";
    case D_ADD:     return "added";
    case D_DEL:     return "removed";
    case D_EDIT:    return "changed";
    }
    
    return "UNKNOWN";
}

rd_diff_t *
diff_get_diff_for_line(rd_file_t *f, int lineno)
{
    assert(f != NULL);
    if (f->diffs == NULL) {
        return NULL;
    }
    
    int i;
    for (i = 0; i < f->n_diffs; i++) {
        rd_diff_t *d = &f->diffs[i];
        if (d->f1_line_start == lineno) {
            return d;
        }
    }
    
    return NULL;
}

int
diff_file_loaded(rd_file_t *f)
{
    assert(f != NULL);
    return f->bytes != NULL;
}

const rd_diff_opts_t *
diff_get_opts(rd_file_t *f)
{
    assert(f != NULL);
    return &f->opts;
}

void
diff_set_opts(rd_file_t *f, rd_diff_opts_t *opts)
{
    assert(f != NULL);
    assert(opts != NULL);
    
    char f1[512], f2[512];
    strncpy(f1, f->f1_name, sizeof(f1));
    strncpy(f2, f->f2_name, sizeof(f2));
    diff_file(f1, f2, f, opts);
}

rd_diff_t *
diff_get_next_diff_for_effective_line(rd_file_t *f, int eff_line)
{
    assert(f != NULL);
    rd_diff_t *d = NULL;
    int i;
    for (i = 0; i < f->n_diffs; i++) {
        d = &f->diffs[i];
        if (d->effective_line > eff_line) {
            return d;
        }
    }
    
    return d;
}

rd_diff_t *
diff_get_prev_diff_for_effective_line(rd_file_t *f, int eff_line)
{
    assert(f != NULL);
    rd_diff_t *d = NULL;
    int i;
    for (i = f->n_diffs - 1; i >= 0; i--) {
        d = &f->diffs[i];
        if (d->effective_line < eff_line) {
            return d;
        }
    }

    return d;
}

void
diff_load_opts_from_prefs(rd_diff_opts_t *opts)
{
    assert(opts != NULL);
    bzero(opts, sizeof(*opts));
    
    opts->ignore_case               = rd_prefs_get_ignore_case();
    opts->ignore_all_whitespace     = rd_prefs_get_ignore_all_whitespace();
    opts->ignore_whitespace_changes = rd_prefs_get_ignore_whitespace_changes();
    opts->ignore_blank_lines        = rd_prefs_get_ignore_blank_lines();
}

int
diff_get_length_for_orig_line(rd_file_t *f, int lineno)
{
    assert(f != NULL);
    if (lineno == -1) {
        return 0;
    }
    assert(lineno >= 0 && lineno < f->n_newlines);
    
    int len = 0;
    if (lineno + 1 < f->n_newlines) {
        // This is NOT the last line.  We can just subtract the byte offsets :-)
        len = f->newlines[lineno + 1] - f->newlines[lineno];
    } else {
        // on the last line, a slightly different calculation
        len = f->n_bytes - f->newlines[lineno];
    }
    
    // Now we need to scan for tabs and add the tab width :-(
    int tw = rd_prefs_get_tab_width();
    int est_len = len;
    if (tw > 0) {
        tw--;
        const char *s = &f->bytes[f->newlines[lineno]];
        int i;
        for (i = 0; i < len; i++) {
            if (s[i] == '\t') {
                est_len += tw;
            }
        }
    }
    
    return est_len;
}

int
diff_get_longest_line_on_left(rd_file_t *f)
{
    assert(f != NULL);
    if (f->longest_left_line > 0) {
        // This is pretty expensive, so just return the cached value if it's available.
        return f->longest_left_line;
    }
    
    int longest = 0;
    int longest_lineno = 0;    // just for debugging
    int i;
    for (i = 0; i < f->n_newlines; i++) {
        int len = diff_get_length_for_orig_line(f, i);
        if (len > longest) {
            longest = len;
            longest_lineno = i;
        }
    }
    
    f->longest_left_line = longest;
    return f->longest_left_line;
}

static int
_diff_orig_line_is_deleted(rd_file_t *f, int lineno)
{
    assert(f != NULL);
    assert(lineno >= 0 && lineno < f->n_newlines);
    
    int i;
    for (i = 0; i < f->n_diffs; i++) {
        rd_diff_t *d = &f->diffs[i];
        int j;
        for (j = 0; j < d->n_deletes; j++) {
            if (lineno == d->deletes[i]) {
                return 1;
            }
        }
    }
    
    return 0;
}

/**
 * This estimates the strlen by adding rd_prefs_get_tab_width() for every \t we find.
 */
static int
_diff_estimate_strlen(const char *s)
{
    int tw = rd_prefs_get_tab_width();
    int len = 0;
    while (*s != '\0') {
        if (*s == '\t') {
            len += tw;
        } else {
            len++;
        }
        
        *s++;
    }
    
    return len;
}

int
diff_get_longest_line_on_right(rd_file_t *f)
{
    assert(f != NULL);
    
    if (f->longest_right_line > 0) {
        // This is pretty expensive, so just return the cached value if it's available.
        return f->longest_right_line;
    }
    
    int longest = 0;
    int i, len;

    // First check for the longest line that we might be adding
    for (i = 0; i < f->n_diffs; i++) {
        rd_diff_t *d = &f->diffs[i];
        int j;
        for (j = 0; j < d->n_changes; j++) {
            len = _diff_estimate_strlen(d->changes[j].line);
            if (len > longest) {
                longest = len;
            }
        }
    }
    
    // Now walk over the left lines, and any that are not deleted, check those.
    for (i = 0; i < f->n_newlines; i++) {
        if (_diff_orig_line_is_deleted(f, i)) {
            continue;
        }

        len = diff_get_length_for_orig_line(f, i);
        if (len > longest) {
            longest = len;
        }        
    }
    
    f->longest_right_line = longest;
    return f->longest_right_line;
}

void
diff_free_file(void)
{
    if (the_file == NULL) {
        return;
    }
    
    rd_file_t *f = the_file;
    assert(f->magic == RD_FILE_MAGIC);
    
    if (f->diffs != NULL) {
        int i;
        for (i = 0; i < f->n_diffs; i++) {
            rd_diff_t *d = &f->diffs[i];
            if (d->changes != NULL) {
                d->n_changes = 0;
                d->n_changes_alloc = 0;
                free(d->changes);
                d->changes = NULL;
            }
            
            if (d->deletes != NULL) {
                d->n_deletes = 0;
                d->n_deletes_alloc = 0;
                free(d->deletes);
                d->deletes = NULL;
            }
            
            bzero(d, sizeof(*d));
        }
        
        f->n_diffs = 0;
        f->n_diffs_alloc = 0;
        free(f->diffs);
    }
    
    if (f->bytes != NULL) {
        f->n_bytes = 0;
        free(f->bytes);
        f->bytes = NULL;
    }
    
    if (f->newlines != NULL) {
        f->n_newlines = 0;
        free(f->newlines);
        f->newlines = NULL;
    }
    
    bzero(f, sizeof(*f));
}