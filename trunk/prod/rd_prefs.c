/*
 *  rd_prefs.c
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include "rd_prefs.h"

typedef struct {
    char path[1024];
    unsigned long last_atime;
} rd_recent_file_t;

#define MAX_NUM_RECENT_FILES 32

typedef struct {
    // Appearance
    int tab_width;              ///< Number of spaces to insert per tab.
    int show_linenos;           ///< Draw the line numbers?
    int font_size;              ///< XXX: Want a more fully feature font selection tool here.
    unsigned long color_add;
    unsigned long color_del;
    unsigned long color_change;
    unsigned long color_background;
    
    // Comparison 
    int ignore_case;            ///< Ignore case differences [diff(1): -i, --ignore-case]
    int ignore_space_changes;   ///< Ignore changes in the amount of white space [diff(1): -b, --ignore-space-change
    int ignore_all_space;       ///< Ignore all white space [diff(1): -w, --ignore-all-space] 
    int ignore_blank_lines;     ///< Ignore blank lines [diff(1): -B --ignore-blank-lines]

    // Recent files
    rd_recent_file_t    recent[MAX_NUM_RECENT_FILES];
    char                last_left[1024];
    char                last_right[1024];
    
    // XXX: multi-monitor support?
} rd_prefs_t;

#define APP_NAME "com.roaringblue.roaringdiff"
#define DEBUG_DUMP() //rd_prefs_debug_dump()

// GLOBALS
static rd_prefs_t g_prefs;

// PROTOTYPES
static void rd_prefs_load(rd_prefs_t *p);
static void rd_prefs_save(rd_prefs_t *p);
static void _rd_prefs_load_recent_files(rd_recent_file_t *p, int num_files, CFStringRef app);
static void _rd_prefs_save_recent_files(rd_recent_file_t *p, int num_files, CFStringRef app);

void
rd_prefs_init(void)
{
    bzero(&g_prefs, sizeof(g_prefs));
    rd_prefs_set_defaults();
    rd_prefs_load(&g_prefs);
}

void
rd_prefs_set_defaults(void)
{
    g_prefs.tab_width = 4;
    g_prefs.show_linenos = 0;
    g_prefs.font_size = 10;
    
    // some nice pastel colors
    g_prefs.color_add = 11796402;
    g_prefs.color_del = 16767189;
    g_prefs.color_change = 9621230;
    g_prefs.color_background = 12437968;

    g_prefs.ignore_case = 0;
    g_prefs.ignore_space_changes = 0;
    g_prefs.ignore_all_space = 0;
    g_prefs.ignore_blank_lines = 0;
}

int
rd_prefs_get_tab_width(void)
{
    return g_prefs.tab_width;
}

int
rd_prefs_get_font_size(void)
{
    return g_prefs.font_size;
}

int
rd_prefs_get_show_linenos(void)
{
    return g_prefs.show_linenos;
}

unsigned long
rd_prefs_get_color_add(void)
{
    return g_prefs.color_add;
}

unsigned long
rd_prefs_get_color_del(void)
{
    return g_prefs.color_del;
}

unsigned long
rd_prefs_get_color_change(void)
{
    return g_prefs.color_change;
}

unsigned long
rd_prefs_get_color_background(void)
{
    return g_prefs.color_background;
}

int
rd_prefs_get_ignore_case(void)
{
    return g_prefs.ignore_case;
}

int
rd_prefs_get_ignore_whitespace_changes(void)
{
    return g_prefs.ignore_space_changes;
}

int
rd_prefs_get_ignore_all_whitespace(void)
{
    return g_prefs.ignore_all_space;
}

int
rd_prefs_get_ignore_blank_lines(void)
{
    return g_prefs.ignore_blank_lines;
}

void
rd_prefs_set_tab_width(int v)
{
    // XXX: Enforce these limits at the UI
    if (v > 0 && v <= 16) {
        g_prefs.tab_width = v;
        DEBUG_DUMP();
        rd_prefs_save(&g_prefs);
    }
}

void
rd_prefs_set_font_size(int v)
{
    // XXX: Enforce these limits at the UI
    if (v >= 4 && v <= 36) {
        g_prefs.font_size = v;
        DEBUG_DUMP();
        rd_prefs_save(&g_prefs);
    }
}

void
rd_prefs_set_show_linenos(int v)
{
    g_prefs.show_linenos = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_color_add(unsigned long v)
{
    g_prefs.color_add = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_color_del(unsigned long v)
{
    g_prefs.color_del = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_color_change(unsigned long v)
{
    g_prefs.color_change = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_color_background(unsigned long v)
{
    g_prefs.color_background = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_ignore_case(int v)
{
    g_prefs.ignore_case = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_ignore_whitespace_changes(int v)
{
    g_prefs.ignore_space_changes = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_ignore_all_whitespace(int v)
{
    g_prefs.ignore_all_space = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_set_ignore_blank_lines(int v)
{
    g_prefs.ignore_blank_lines = v;
    DEBUG_DUMP();
    rd_prefs_save(&g_prefs);
}

void
rd_prefs_debug_dump(void)
{
    fprintf(stderr, "PREFS DUMP\n");
    fprintf(stderr, "  .tab_width            = %d\n", g_prefs.tab_width);
    fprintf(stderr, "  .show_linenos         = %d\n", g_prefs.show_linenos);
    fprintf(stderr, "  .font_size            = %d\n", g_prefs.font_size);
    fprintf(stderr, "  .color_add            = %08x\n", (unsigned int) g_prefs.color_add);
    fprintf(stderr, "  .color_del            = %08x\n", (unsigned int) g_prefs.color_del);
    fprintf(stderr, "  .color_change         = %08x\n", (unsigned int) g_prefs.color_change);
    fprintf(stderr, "  .color_background     = %08x\n", (unsigned int) g_prefs.color_background);
    fprintf(stderr, "  .ignore_case          = %d\n", g_prefs.ignore_case);
    fprintf(stderr, "  .ignore_space_changes = %d\n", g_prefs.ignore_space_changes);
    fprintf(stderr, "  .ignore_all_space     = %d\n", g_prefs.ignore_all_space);
    fprintf(stderr, "  .ignore_blank_lines   = %d\n", g_prefs.ignore_blank_lines);

    fprintf(stderr, "DONE\n");
}

static void
_rd_prefs_save_singleton_int(const char *key, int val, CFStringRef app)
{
    assert(key != NULL);

    CFStringRef k = CFStringCreateWithCString(kCFAllocatorSystemDefault, key, kCFStringEncodingASCII);
    CFNumberRef n = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &val);
    
    if (k != NULL && n != NULL) {
        CFPreferencesSetAppValue(k, n, app);
    } else {
        fprintf(stderr, "Error saving pref value: %s = %d\n", key, val);
    }

    CFRelease(k);
    CFRelease(n);
}

static void
_rd_prefs_save_singleton_string(const char *key, const char *val, CFStringRef app)
{
    assert(key != NULL);

    CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingASCII);
    CFStringRef n = CFStringCreateWithCString(NULL, val, kCFStringEncodingUTF8);
    
    if (k != NULL && n != NULL) {
        CFPreferencesSetAppValue(k, n, app);
    } else {
        fprintf(stderr, "Error saving pref value: %s = %s\n", key, val);
    }

    CFRelease(k);
    CFRelease(n);
}

static void
rd_prefs_save(rd_prefs_t *p)
{
    assert(p != NULL);

    CFStringRef app = CFSTR(APP_NAME);
    
    _rd_prefs_save_singleton_int("tab_width",             p->tab_width,             app);
    _rd_prefs_save_singleton_int("show_linenos",          p->show_linenos,          app);
    _rd_prefs_save_singleton_int("font_size",             p->font_size,             app);

    _rd_prefs_save_singleton_int("ignore_case",           p->ignore_case,           app);
    _rd_prefs_save_singleton_int("ignore_space_changes",  p->ignore_space_changes,  app);
    _rd_prefs_save_singleton_int("ignore_all_space",      p->ignore_all_space,      app);
    _rd_prefs_save_singleton_int("ignore_blank_lines",    p->ignore_blank_lines,    app);

    _rd_prefs_save_singleton_int("color_add",             p->color_add,             app);
    _rd_prefs_save_singleton_int("color_del",             p->color_del,             app);
    _rd_prefs_save_singleton_int("color_change",          p->color_change,          app);
    _rd_prefs_save_singleton_int("color_background",      p->color_background,      app);
    
    _rd_prefs_save_recent_files(p->recent, MAX_NUM_RECENT_FILES, app);
    
    _rd_prefs_save_singleton_string("last_left",          p->last_left,             app);
    _rd_prefs_save_singleton_string("last_right",         p->last_right,            app);

    (void) CFPreferencesAppSynchronize(app);
}

/**
 * If the value could not be loaded, the value in *val is not changed
 */
static void
_rd_prefs_load_singleton_int(const char *key, int *val, CFStringRef app)
{
    assert(key != NULL);
    assert(val != NULL);
    
    CFStringRef k = CFStringCreateWithCString(kCFAllocatorSystemDefault, key, kCFStringEncodingASCII);
    if (k == NULL) {
        return;
    }

    CFNumberRef n = CFPreferencesCopyAppValue(k, app);
    if (n != NULL) {
        int temp = 0;
        if (!CFNumberGetValue(n, kCFNumberIntType, &temp)) {
            // Oh well
        } else {
            *val = temp;
        }
        
        CFRelease(n);
    }
}

static void
_rd_prefs_load_singleton_string(const char *key, char *val, size_t sizeof_val, CFStringRef app)
{
    assert(key != NULL);
    assert(val != NULL);
    
    bzero(val, sizeof_val);
    
    CFStringRef k = CFStringCreateWithCString(kCFAllocatorSystemDefault, key, kCFStringEncodingASCII);
    if (k == NULL) {
        return;
    }
    
    CFStringRef v = (CFStringRef) CFPreferencesCopyAppValue(k, app);
    if (v != NULL) {
        if (CFStringGetCString(v, val, sizeof_val, kCFStringEncodingUTF8)) {
            // success
        }
        
        CFRelease(v);
    }    
}

static void
rd_prefs_load(rd_prefs_t *p)
{
    assert(p != NULL);

    CFStringRef app = CFSTR(APP_NAME);
    
    _rd_prefs_load_singleton_int("tab_width",             &p->tab_width,    app);
    _rd_prefs_load_singleton_int("show_linenos",          &p->show_linenos, app);
    _rd_prefs_load_singleton_int("font_size",             &p->font_size,    app);

    _rd_prefs_load_singleton_int("ignore_case",           &p->ignore_case,          app);
    _rd_prefs_load_singleton_int("ignore_space_changes",  &p->ignore_space_changes, app);
    _rd_prefs_load_singleton_int("ignore_all_space",      &p->ignore_all_space,     app);
    _rd_prefs_load_singleton_int("ignore_blank_lines",    &p->ignore_blank_lines,   app);

    _rd_prefs_load_singleton_int("color_add",             (int*) &p->color_add,        app);
    _rd_prefs_load_singleton_int("color_del",             (int*) &p->color_del,        app);
    _rd_prefs_load_singleton_int("color_change",          (int*) &p->color_change,     app);
    _rd_prefs_load_singleton_int("color_background",      (int*) &p->color_background, app);

    // load recent files
    _rd_prefs_load_recent_files(p->recent, MAX_NUM_RECENT_FILES, app);

    _rd_prefs_load_singleton_string("last_left",    p->last_left,  sizeof(p->last_left),  app);
    _rd_prefs_load_singleton_string("last_right",   p->last_right, sizeof(p->last_right), app);
}

static void
_rd_prefs_load_recent_files(rd_recent_file_t *r, int num_files, CFStringRef app)
{
    assert(r != NULL);
    assert(app != NULL);

    CFArrayRef the_array = CFPreferencesCopyAppValue(CFSTR("recent_files"), app);
    if (the_array == NULL) {
        return;
    }
    
    CFIndex array_count = CFArrayGetCount(the_array);        
    int i;
    for (i = 0; i < num_files; i++) {
        rd_recent_file_t *f = &r[i];
        if (i >= array_count) {
            bzero(f, sizeof(*f));
        } else {
            const void *f_val    = CFArrayGetValueAtIndex(the_array, i);
            if (CFGetTypeID(f_val) == CFStringGetTypeID()) {
                // string.  set the time to now.
                if (CFStringGetCString(f_val, f->path, sizeof(f->path), kCFStringEncodingUTF8)) {
                    // success 
                    f->path[sizeof(f->path) - 1] = '\0';
                    f->last_atime = (unsigned long) time(NULL);
                }
            } else if (CFGetTypeID(f_val) == CFDictionaryGetTypeID()) {
                CFStringRef f_name  = CFDictionaryGetValue(f_val, CFSTR("name"));
                CFNumberRef f_atime = CFDictionaryGetValue(f_val, CFSTR("atime"));
                
                if (CFStringGetCString(f_name, f->path, sizeof(f->path), kCFStringEncodingUTF8)) {
                    // success
                    f->path[sizeof(f->path) - 1] = '\0';
                
                    if (CFNumberGetValue(f_atime, kCFNumberLongType, (long*) &f->last_atime)) {
                        // success
                    } else {
                        f->last_atime = (unsigned long) time(NULL);
                    }
                }
            }
        }
    }
    
    CFRelease(the_array);
}

static void
_rd_prefs_save_recent_files(rd_recent_file_t *r, int num_files, CFStringRef app)
{
    assert(r != NULL);
    assert(app != NULL);

    CFMutableArrayRef the_array = CFArrayCreateMutable(NULL, num_files, &kCFTypeArrayCallBacks);
    assert(the_array != NULL);
    
    int i;
    
    for (i = 0; i < num_files; i++) {
        rd_recent_file_t *f = &r[i];
        if (f->path[0] == '\0') {
            continue;
        }

        CFMutableDictionaryRef f_dict = CFDictionaryCreateMutable(NULL, 2, NULL, NULL);
        assert(f_dict != NULL);

        CFStringRef f_name = CFStringCreateWithCString(NULL, f->path, kCFStringEncodingUTF8);
        assert(f_name != NULL);

        CFNumberRef f_atime = CFNumberCreate(NULL, kCFNumberLongType, &f->last_atime);
        assert(f_atime != NULL);
        
        CFDictionaryAddValue(f_dict, CFSTR("name"),  f_name);
        CFDictionaryAddValue(f_dict, CFSTR("atime"), f_atime);
        
        CFArraySetValueAtIndex(the_array, (CFIndex) i, f_dict);
    }
    
    CFPreferencesSetAppValue(CFSTR("recent_files"), the_array, app);
    CFRelease(the_array);
}

void
rd_prefs_file_add_recent(const char *path)
{
    int i;
    unsigned long time_now = (unsigned long) time(NULL);
    rd_recent_file_t *r = NULL;
    
    // XXX: Check for whether the file is already in the list, and if so, update its timestamp
    for (i = 0; i < MAX_NUM_RECENT_FILES; i++) {
        r = &g_prefs.recent[i];
        if (strncmp(r->path, path, sizeof(r->path)) == 0) {
            r->last_atime = time_now;
            rd_prefs_save(&g_prefs);
            return;
        }
    }
    
    // OK, we need to add the file... see if there's enough room
    for (i = 0; i < MAX_NUM_RECENT_FILES; i++) {
        r = &g_prefs.recent[i];
        if (r->path[0] == '\0') {
            strncpy(r->path, path, sizeof(r->path));
            r->path[sizeof(r->path) - 1] = '\0';
            r->last_atime = time_now;
            rd_prefs_save(&g_prefs);
            return;
        }
    }
    
    // We didn't have room; remove the oldest used file, which may not be the first one!
    int oldest_i = 0;
    for (i = 1; i < MAX_NUM_RECENT_FILES; i++) {
        r = &g_prefs.recent[i];
        if (r->last_atime < g_prefs.recent[oldest_i].last_atime) {
            oldest_i = i;
        }
    }
    
    r = &g_prefs.recent[oldest_i];
    strncpy(r->path, path, sizeof(r->path));
    r->path[sizeof(r->path) - 1] = '\0';
    r->last_atime = time_now;
    rd_prefs_save(&g_prefs);
}

int
rd_prefs_file_get_num_recent(void)
{
    int num = 0;
    int i;
    for (i = 0; i < MAX_NUM_RECENT_FILES; i++) {
        rd_recent_file_t *r = &g_prefs.recent[i];
        if (r->path[0] == '\0') {
            break;
        }
        num++;
    }
    
    return num;
}

const char *
rd_prefs_file_get_recent_at_index(int idx)
{
    if (idx < 0 || idx >= MAX_NUM_RECENT_FILES) {
        return NULL;
    }
    
    return g_prefs.recent[idx].path;
}

void
rd_prefs_set_last_left_file(const char *s)
{
    strncpy(g_prefs.last_left, s, sizeof(g_prefs.last_left));
}

void
rd_prefs_set_last_right_file(const char *s)
{
    strncpy(g_prefs.last_right, s, sizeof(g_prefs.last_right));
}

const char *
rd_prefs_get_last_left_file(void)
{
    return g_prefs.last_left;
}

const char *
rd_prefs_get_last_right_file(void)
{
    return g_prefs.last_right;
}

#if 0
/**
 * Filters are used to convert binary files to text before comparing them.
 */
typedef struct {
    char extension[32];         /**< File extension */
    char user_filter[512];      /**< Filter set by user */
    char default_filter[512];   /**< Filter default */
    int compress_whitespace;    /**< XXX: Apple's FileMerge has this option. */
    int ignore_case;            /**< XXX: Apple's FileMerge has this option. */
} rd_filter_t;

/**
 * When comparing directories, some file names we want to skip.
 * (e.g., CVS, .cvs, .svn, tags, .o, .out)
 */
typedef struct {
    char regexp[64];            /**< XXX: */
} rd_dir_skip_file_t;
#endif
