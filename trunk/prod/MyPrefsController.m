/*
 *  MyPrefsController.m
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

#import "MyPrefsController.h"
#include "rd_prefs.h"
#include "qd_utils.h"

@implementation MyPrefsController

- (IBAction)editIgnoreCase:(id)sender
{
    (void) sender;
    rd_prefs_set_ignore_case([ignoreCase intValue]);
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editIgnoreWhitespaceChanges:(id)sender
{
    (void) sender;
    rd_prefs_set_ignore_whitespace_changes([ignoreWhitespaceChanges intValue]);
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editIgnoreAllWhitespace:(id)sender
{
    (void) sender;
    rd_prefs_set_ignore_all_whitespace([ignoreAllWhitespace intValue]);
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editIgnoreBlankLines:(id)sender
{
    (void) sender;
    rd_prefs_set_ignore_blank_lines([ignoreBlankLines intValue]);
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editColorAdd:(id)sender
{
    (void) sender;
    rd_prefs_set_color_add(qd_utils_int_from_nscolor([colorAdd color]));
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editColorBackground:(id)sender
{
    (void) sender;
    rd_prefs_set_color_background(qd_utils_int_from_nscolor([colorBackground color]));
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editColorChange:(id)sender
{
    (void) sender;
    rd_prefs_set_color_change(qd_utils_int_from_nscolor([colorChange color]));
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editColorDel:(id)sender
{
    (void) sender;
    rd_prefs_set_color_del(qd_utils_int_from_nscolor([colorDel color]));
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editFontSize:(id)sender
{
    (void) sender;
    rd_prefs_set_font_size([fontSize intValue]);
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)editTabWidth:(id)sender
{
    (void) sender;
    rd_prefs_set_tab_width([tabWidth intValue]);
    [diffWindow setNeedsDisplay:YES];
}

- (IBAction)toggleShowLineNos:(id)sender
{
    (void) sender;
    rd_prefs_set_show_linenos([showLineNos intValue]);
    [diffWindow setNeedsDisplay:YES];
}

- (void)awakeFromNib
{
    [showLineNos     setIntValue:rd_prefs_get_show_linenos()];
    [tabWidth        setIntValue:rd_prefs_get_tab_width()];
    [fontSize        setIntValue:rd_prefs_get_font_size()];
    
    [ignoreCase                 setIntValue:rd_prefs_get_ignore_case()];
    [ignoreWhitespaceChanges    setIntValue:rd_prefs_get_ignore_whitespace_changes()];
    [ignoreAllWhitespace        setIntValue:rd_prefs_get_ignore_all_whitespace()];
    [ignoreBlankLines           setIntValue:rd_prefs_get_ignore_blank_lines()];

    [colorAdd        setColor:qd_utils_nscolor_from_int(rd_prefs_get_color_add())];
    [colorDel        setColor:qd_utils_nscolor_from_int(rd_prefs_get_color_del())];
    [colorChange     setColor:qd_utils_nscolor_from_int(rd_prefs_get_color_change())];
    [colorBackground setColor:qd_utils_nscolor_from_int(rd_prefs_get_color_background())];    
}
@end
