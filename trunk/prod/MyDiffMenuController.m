/*
 *  MyDiffMenuController.m
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

#import "MyDiffMenuController.h"
#include "diff.h"

extern rd_file_t g_file;

static void
my_setup_change_menu(NSMenu *menu, id target)
{
	assert(menu != NULL);
	int i;
	
	/* First remove all the items */
	int num_items = [menu numberOfItems];
	for (i = 0; i < num_items; i++) {
		[menu removeItemAtIndex:0];
	}
	
	if (g_file.n_diffs <= 0) {
		/* No changes */
		NSMenuItem *dummyItem = [[NSMenuItem alloc] initWithTitle:@"(no changes)" action:NULL keyEquivalent:@""];
		[dummyItem setEnabled:NO];	/* XXX: This doesn't seem to be sticky */
		[menu addItem:dummyItem];
		return;
	}

	/* Add in the new items */
	char label[128];
	
	for (i = 0; i < g_file.n_diffs; i++) {
		rd_diff_t *d = &g_file.diffs[i];
		int num_lines = d->d_type == D_ADD ? d->f2_n_lines : d->f1_n_lines;
		snprintf(label,
				 sizeof(label),
				 "%d: %s %d line%s",
				 d->f1_line_start + 1,
				 diff_type_to_string(d->d_type),
				 num_lines,
				 num_lines == 1 ? "" : "s");
		NSString *nss_label = [[NSString alloc] initWithCString:label];
		NSMenuItem *theItem = [[NSMenuItem alloc] initWithTitle:nss_label action:NULL keyEquivalent:@""];
		[theItem setTag:d->effective_line];

		/* Set up the target & message to call when the item is selected.  Here, we'll call the quickdraw view */
		[theItem setTarget:target];
		[theItem setAction:@selector(menuSelect:)];
		
		/* Finally add the item to the menu */
		[menu addItem:theItem];
	}

	/* Add a disabled indicator of how many changes are in the file */
	/* This is now in the window title :-) */
//	[menu addItem:[NSMenuItem separatorItem]];
//	snprintf(label, sizeof(label), "%d Changes", g_file.n_diffs);
//	[menu addItem:[[NSMenuItem alloc] initWithTitle:[[NSString alloc] initWithCString:label] action:NULL keyEquivalent:@""]];
}

@implementation MyDiffMenuController

- (IBAction)menuSelect:(id)sender
{
	// OK, let's scroll the diffView to the given line
	[diffView scrollToLine:[sender tag]];
}

- (IBAction)menuInit
{
	my_setup_change_menu(menuData, self);
}

@end
