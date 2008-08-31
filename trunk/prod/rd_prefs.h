/*
 *  rd_prefs.h
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

#ifndef _RD_PREFS_H
#define _RD_PREFS_H

/**
 * Initialize the preferences system
 */
void
rd_prefs_init(void);

/**
 * Reset the preferences to default values.
 */
void
rd_prefs_set_defaults(void);

/**
 * Returns the tab width
 */
int
rd_prefs_get_tab_width(void);

void
rd_prefs_set_tab_width(int v);

int
rd_prefs_get_font_size(void);

void
rd_prefs_set_font_size(int v);

int
rd_prefs_get_show_linenos(void);

void
rd_prefs_set_show_linenos(int v);

unsigned long
rd_prefs_get_color_add(void);

void
rd_prefs_set_color_add(unsigned long v);

unsigned long
rd_prefs_get_color_del(void);

int
rd_prefs_get_ignore_case(void);

int
rd_prefs_get_ignore_whitespace_changes(void);

int
rd_prefs_get_ignore_all_whitespace(void);

int
rd_prefs_get_ignore_blank_lines(void);

void
rd_prefs_set_color_del(unsigned long v);

unsigned long
rd_prefs_get_color_change(void);

void
rd_prefs_set_color_change(unsigned long v);

unsigned long
rd_prefs_get_color_background(void);

void
rd_prefs_set_color_background(unsigned long v);

void
rd_prefs_set_ignore_case(int v);

void
rd_prefs_set_ignore_whitespace_changes(int v);

void
rd_prefs_set_ignore_all_whitespace(int v);

void
rd_prefs_set_ignore_blank_lines(int v);

void
rd_prefs_debug_dump(void);

void
rd_prefs_file_add_recent(const char *path);

int
rd_prefs_file_get_num_recent(void);

const char *
rd_prefs_file_get_recent_at_index(int idx);

void
rd_prefs_set_last_left_file(const char *s);

void
rd_prefs_set_last_right_file(const char *s);

const char *
rd_prefs_get_last_left_file(void);

const char *
rd_prefs_get_last_right_file(void);


/**
 * Returns the filter for the given file.
 *
 * Given the file name, look at its extension and see if we want to run a filter program
 * on the file before we compare it.
 *
 * @param file (i) File name.
 * return rd_filter_t object if a filter was found, otherwise NULL.  Do NOT free this!
 */
#if 0
rd_filter_t *
rd_prefs_get_filter_for_file(const char *file);
#endif

#endif
