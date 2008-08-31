/*
 *  MyQuickDrawView.h
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
#include "MyHorzCompareView.h"
#include "MyDiffVertScroller.h"
#include "MyDiffHorzScroller.h"

typedef struct {
    int num_lines;       ///< Number of lines that will fit onto the view (visible) 
    int selected_line;   ///< screen coordinates
    int effective_line;  ///< The effective line number we're on within the sum of the two files 
    int effective_start; ///< The effective line number that is the first line on the screen 

    int num_cols;        ///< Number of columns that fit into either the left or right view (visible) 
    int left_col_start;  ///< The left column to start drawing 
    int right_col_start; ///< The right column to start drawing 
} qd_diff_view_t;

@interface MyQuickDrawView : NSQuickDrawView
{
    IBOutlet MyHorzCompareView *horzView;
    IBOutlet MyDiffVertScroller *vertScroller;
    IBOutlet MyDiffHorzScroller *leftHorzScroller, *rightHorzScroller;
    qd_diff_view_t m_view;
    const char *m_sel_left, *m_sel_right;
    int mouse_click_x, mouse_click_y;
}

// Drawing
- (void)drawRect:(NSRect)rect;
- (void)tweakFrame;

- (id)initWithFrame:(NSRect)frame;
- (void)doVertScroll:(id)sender;
- (void)doHorzScroll:(MyDiffHorzScroller *)scroller;

// Events
- (void)mouseDown:(NSEvent *)theEvent;
- (void)keyDown:(NSEvent *)theEvent;
- (void)scrollWheel:(NSEvent *)theEvent;

// Menu selected event on the window.
- (void)scrollToLine:(int)lineno;

// Member functions
- (int)selectedLine;
- (const char*)selectedLeftLine;
- (const char*)selectedRightLine;
- (void)setSelectedLeftLine:(const char *)s;
- (void)setSelectedRightLine:(const char *)s;

- (void)getQuickDrawBoundsRect:(Rect *)r;

// Navigation
- (BOOL)canJumpNextChange;
- (BOOL)canJumpPrevChange;
- (void)jumpNextChange;
- (void)jumpPrevChange;
- (BOOL)canJumpNextLine;
- (BOOL)canJumpPrevLine;
- (void)jumpNextLine;
- (void)jumpPrevLine;

-(qd_diff_view_t*)m_view;

@end
