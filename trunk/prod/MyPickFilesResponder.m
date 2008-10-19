/*
 *  MyPickFilesResponder.m
 *  roaringdiff - http://www.biscade.com/tools/diff/

/* 
 * Copyright ¬© 2006-2008 Mitch Haile.
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

#import "MyPickFilesResponder.h"
#include "diff.h"
#include "rd_prefs.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

static void
update_message_for_file(NSComboBox *ns_filename, NSTextField *msg_field)
{
    assert(ns_filename != NULL);
    assert(msg_field != NULL);
    
    char msg[256];
    char f1[256];
    struct stat sbuf;

    [[ns_filename stringValue] getCString:f1 maxLength:sizeof(f1)];

    // XXX: this stat is not working for some reason
    if (stat(f1, &sbuf) == 0) {
        snprintf(msg, sizeof(msg), "File %s is %d bytes.", f1, (int) sbuf.st_size);
    } else {
        snprintf(msg, sizeof(msg), "Unable to stat file (%s).", strerror(errno));
    }
    
    [msg_field setStringValue:[[NSString alloc] initWithCString:msg]];
}

static void
select_file(NSComboBox *ns_field)
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
 
    [oPanel setAllowsMultipleSelection:NO];
    int result = [oPanel runModalForDirectory:NSHomeDirectory() file:nil types:nil];
    if (result == NSOKButton) {
        NSArray *filesToOpen = [oPanel filenames];
        if ([filesToOpen count] == 1) {
            NSString *file = [filesToOpen objectAtIndex:0];
            [ns_field setStringValue:file];
        }
    }
}

@implementation MyPickFilesResponder

extern int get_cmd_files(const char **f1, const char **f2, const char **left_label, const char **right_label);

- (void)awakeFromNib
{
    const char *f1 = NULL, *f2 = NULL, *left_label = NULL, *right_label = NULL;
    int got_cmd_line_files = get_cmd_files(&f1, &f2, &left_label, &right_label);
    
    if (!got_cmd_line_files) {
        f1 = rd_prefs_get_last_left_file();
        f2 = rd_prefs_get_last_right_file();
    }
    
    [leftFileName  setStringValue:[[NSString alloc] initWithCString:f1]];
    [rightFileName setStringValue:[[NSString alloc] initWithCString:f2]];
    
    update_message_for_file(leftFileName,  leftFileStats);
    update_message_for_file(rightFileName, rightFileStats);

    if (got_cmd_line_files) {
        [self compareFiles:self leftLabel:left_label rightLabel:right_label];
        // Bring app to front when started from command line
        [NSApp activateIgnoringOtherApps:YES];
    }
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    (void) alert;
    (void) returnCode;
    (void) contextInfo;

    // XXX:
}

- (IBAction)buttonCompareFiles:(id)sender
{
    [self compareFiles:sender leftLabel:NULL rightLabel:NULL];
}

- (IBAction)compareFiles:(id)sender leftLabel:(char*)leftLabel rightLabel:(char*)rightLabel
{
    char f1[256];
    char f2[256];

    [[leftFileName stringValue]  getCString:f1 maxLength:sizeof(f1)];
    [[rightFileName stringValue] getCString:f2 maxLength:sizeof(f2)];
    
    rd_prefs_set_last_left_file(f1);
    rd_prefs_set_last_right_file(f2);
    
    rd_prefs_file_add_recent(f1);
    rd_prefs_file_add_recent(f2);
    
    rd_diff_opts_t opts;
    diff_load_opts_from_prefs(&opts);
    diff_file(f1, f2, &g_file, &opts);
    
    // Set up the title of the window
    char title[256];
    snprintf(title,
             sizeof(title),
             "RoaringDiff: %s <-> %s (%d difference%s)",
             leftLabel != NULL  ? leftLabel  : f1,
             rightLabel != NULL ? rightLabel : f2,
             g_file.n_diffs,
             g_file.n_diffs == 1 ? "" : "s");
    [diffWindow setTitle:[[NSString alloc] initWithCString:title]];
    
    /*
     * HACK:  By default, "maximize" the window on the parent screen.
     *        To do this, just set the size of the window to the size on the parent screen. 
     *        The window server takes care of the dock and menubar.
     */
    NSRect my_rect = [diffWindow frame];
    my_rect.size = [[diffWindow screen] frame].size;
    [diffWindow setFrame:my_rect display:YES];

    // Show the diff window if it is invisible.
    [diffWindow orderFront:sender];
    [diffWindow makeMainWindow];
    [diffWindow makeKeyAndOrderFront:nil];
    
    // Setup the changeMenu.
    [changeMenuController menuInit];
    
    /*
     * HACK:  This is a bit silly.  We pretend the diff worked, then we check and see what happened--so that we can put a sheet up on the diff window
     *        Ideally, we'd put the sheet error on the Choose Files window.
     */
    if (!diff_is_supported()) {
        NSAlert *alert = [[[NSAlert alloc] init] autorelease];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:@"Unable to compare the files"];
        [alert setInformativeText:@"RoaringDiff requires the Apple Developer Tools.  Please visit http://developer.apple.com/ to get them."];
        [alert setAlertStyle:NSCriticalAlertStyle];
        
        [alert beginSheetModalForWindow:diffWindow modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:nil];
    }
    
    // We need to set that we want the view to be displayed too 
    [diffView setNeedsDisplay:YES];
}

- (IBAction)leftFileSelect:(id)sender
{
    (void) sender;
    select_file(leftFileName);
    update_message_for_file(leftFileName, leftFileStats);
}

- (IBAction)leftFileTextEntry:(id)sender
{
    (void) sender;
    update_message_for_file(leftFileName, leftFileStats);
}

- (IBAction)rightFileSelect:(id)sender
{
    (void) sender;
    select_file(rightFileName);
    update_message_for_file(rightFileName, rightFileStats);
}

- (IBAction)rightFileTextEntry:(id)sender
{
    (void) sender;
    update_message_for_file(rightFileName, rightFileStats);
}

// - (unsigned int)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString
// - (NSString *)comboBox:(NSComboBox *)aComboBox completedString:(NSString *)uncompletedString

- (int)numberOfItemsInComboBox:(NSComboBox *)aComboBox
{
    int num = rd_prefs_file_get_num_recent();
    [aComboBox setNumberOfVisibleItems:num];
    return num;
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(int)idx
{
    (void) aComboBox;
    const char *s = rd_prefs_file_get_recent_at_index(idx);
    return [[NSString alloc] initWithCString:s];
}

@end
