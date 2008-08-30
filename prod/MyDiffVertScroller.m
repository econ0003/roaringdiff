/*
 *  MyDiffVertScroller.m
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

#import "MyDiffVertScroller.h"
#include "diff.h"
#include "qd_utils.h"

#define USE_CUSTOM_COLORS 0

@implementation MyDiffVertScroller

- (id)initWithFrame:(NSRect)frame
{
	[super initWithFrame:frame];
	[self setEnabled:YES]; 
	[self setTarget:self];
	[self setAction:@selector(doVertScroll:)];
	
	return self;
}

- (void)doVertScroll:(id)sender 
{
	[diffView doVertScroll:sender];
}

- (void)drawRect:(NSRect) rect
{
	[super drawRect:rect];
	
#if USE_CUSTOM_COLORS
	/* XXX it's possible for the user to change these colors... so they may not be the same on every draw */
	NSColor *nscol_add    = qd_utils_nscolor_from_int(rd_prefs_get_color_add()),
			*nscol_del    = qd_utils_nscolor_from_int(rd_prefs_get_color_del()),
			*nscol_change = qd_utils_nscolor_from_int(rd_prefs_get_color_change());
#else
	NSColor *nscol = qd_utils_nscolor_from_int(0x00666666);
#endif
	NSRect dr;
	NSRect slotRect = [self rectForPart:NSScrollerKnobSlot];
	/* There is some decorative stuff at both ends of the knob slot that the slotRect includes.  Cull that area */
	slotRect.origin.y += 8.0f;
	slotRect.size.height -= 16.0f;

#if USE_CUSTOM_COLORS
	dr.size.width = slotRect.size.width - 10.0f;
	dr.origin.x = slotRect.origin.x + 5.0f;
#else
	dr.size.width = slotRect.size.width - 8.0f;
	dr.origin.x = slotRect.origin.x + 4.0f;
#endif

	/* Draw those lines! */
	int i;
	for (i = 0; i < g_file.n_diffs; i++) {
		rd_diff_t *d = &g_file.diffs[i];
		float relative_pos = ((float) d->effective_line) / ((float) g_file.n_effective_lines);
		float relative_height = ((float) (d->n_changes + d->n_deletes)) / ((float) g_file.n_effective_lines);
		dr.origin.y = slotRect.origin.y + (relative_pos * slotRect.size.height);
		dr.size.height = relative_height * slotRect.size.height;
			
#if USE_CUSTOM_COLORS
		switch (d->d_type) {
		case D_ADD:
			[nscol_add set];
			break;
		case D_DEL:
			[nscol_del set];
			break;
		case D_EDIT:
			[nscol_change set];
			break;
		}
#else
		[nscol set];
#endif
		NSRectFill(dr);
	}
	
	/* Redraw the knob over our mess */
	[self drawKnob];
}

@end
