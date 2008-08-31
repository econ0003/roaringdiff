/*
 *  MyCompareMenuController.m
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

#import "MyCompareMenuController.h"
#include <stdio.h>
#include "diff.h"

@implementation MyCompareMenuController

- (BOOL)validateMenuItem:(NSMenuItem *)item {
    SEL action = [item action];

    // Set checkmarks on items as appropriate
    const rd_diff_opts_t *o = diff_get_opts(&g_file);
    assert(o != NULL);
    
    if (action == @selector(ignoreCase:)) {
        [item setState:(o->ignore_case ? NSOnState : NSOffState)];
    } else if (action == @selector(ignoreAllWhitespace:)) {
        [item setState:(o->ignore_all_whitespace ? NSOnState : NSOffState)];
    } else if (action == @selector(ignoreWhitespaceChanges:)) {
        [item setState:(o->ignore_whitespace_changes ? NSOnState : NSOffState)];
    } else if (action == @selector(ignoreBlankLines:)) {
        [item setState:(o->ignore_blank_lines ? NSOnState : NSOffState)];
    } else if (action == @selector(nextLine:)) {
        return [diffView canJumpNextLine];
    } else if (action == @selector(prevLine:)) {
        return [diffView canJumpPrevLine];
    } else if (action == @selector(nextChange:)) {
        return [diffView canJumpNextChange];
    } else if (action == @selector(prevChange:)) {
        return [diffView canJumpPrevChange];
    }

    return YES;
}

- (IBAction)chooseNewLeft:(id)sender
{
    (void) sender;

    [pickFilesResponder leftFileSelect:self];
}

- (IBAction)chooseNewRight:(id)sender
{
    (void) sender;

    [pickFilesResponder rightFileSelect:self];
}

- (IBAction)ignoreAllWhitespace:(id)sender
{
    (void) sender;

    // XXX: There's no need for this to be so big
    rd_diff_opts_t o;
    memcpy(&o, diff_get_opts(&g_file), sizeof(o));
    o.ignore_all_whitespace = ! o.ignore_all_whitespace;
    diff_set_opts(&g_file, &o);
}

- (IBAction)ignoreBlankLines:(id)sender
{
    (void) sender;

    // XXX: There's no need for this to be so big
    rd_diff_opts_t o;
    memcpy(&o, diff_get_opts(&g_file), sizeof(o));
    o.ignore_blank_lines = ! o.ignore_blank_lines;
    diff_set_opts(&g_file, &o);
}

- (IBAction)ignoreCase:(id)sender
{
    (void) sender;

    // XXX: There's no need for this to be so big
    rd_diff_opts_t o;
    memcpy(&o, diff_get_opts(&g_file), sizeof(o));
    o.ignore_case = ! o.ignore_case;
    diff_set_opts(&g_file, &o);
}

- (IBAction)ignoreWhitespaceChanges:(id)sender
{
    (void) sender;

    // XXX: There's no need for this to be so big
    rd_diff_opts_t o;
    memcpy(&o, diff_get_opts(&g_file), sizeof(o));
    o.ignore_whitespace_changes = ! o.ignore_whitespace_changes;
    diff_set_opts(&g_file, &o);
}

- (IBAction)reverseFiles:(id)sender
{
    (void) sender;

    char f1[512], f2[512];
    strncpy(f1, g_file.f1_name, sizeof(f1));
    strncpy(f2, g_file.f2_name, sizeof(f2));
    
    rd_diff_opts_t opts;
    memcpy(&opts, diff_get_opts(&g_file), sizeof(opts));
    
    diff_file(f2, f1, &g_file, &opts);
    
    [diffView setNeedsDisplay:YES];
    [horzView setNeedsDisplay:YES];
}

- (IBAction)reloadDiff:(id)sender
{
    (void) sender;

    char f1[512], f2[512];
    strncpy(f1, g_file.f1_name, sizeof(f1));
    strncpy(f2, g_file.f2_name, sizeof(f2));
    
    rd_diff_opts_t opts;
    memcpy(&opts, diff_get_opts(&g_file), sizeof(opts));
    
    diff_file(f1, f2, &g_file, &opts);
    
    [diffView setNeedsDisplay:YES];
    [horzView setNeedsDisplay:YES];
}

- (IBAction)nextLine:(id)sender
{
    (void) sender;

    [diffView jumpNextLine];
}

- (IBAction)nextChange:(id)sender
{
    (void) sender;

    [diffView jumpNextChange];
}

- (IBAction)prevLine:(id)sender
{
    (void) sender;

    [diffView jumpPrevLine];
}

- (IBAction)prevChange:(id)sender
{
    (void) sender;

    [diffView jumpPrevChange];
}

@end
