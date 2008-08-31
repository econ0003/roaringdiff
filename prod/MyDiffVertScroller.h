/*
 *  MyDiffVertScroller.h
 *  roaringdiff - http://www.biscade.com/tools/diff/

/* 
 * Copyright � 2006-2008 Mitch Haile.
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

@interface MyDiffVertScroller : NSScroller
{
    IBOutlet id diffView;
    NSColor *nscol_add;
    NSColor *nscol_del;
    NSColor *nscol_change;
}

- (id)initWithFrame:(NSRect)frame;
- (void)drawRect:(NSRect) rect;
- (void)doVertScroll:(id)sender;

@end
