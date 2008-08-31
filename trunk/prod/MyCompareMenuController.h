/*
 *  MyCompareMenuController.h
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

#import <Cocoa/Cocoa.h>

#include "MyPickFilesResponder.h"
#include "MyQuickDrawView.h"

@interface MyCompareMenuController : NSObject
{
    IBOutlet MyQuickDrawView *diffView;
    IBOutlet id horzView;
    IBOutlet MyPickFilesResponder *pickFilesResponder;
}

- (BOOL)validateMenuItem:(NSMenuItem *)item;

- (IBAction)chooseNewLeft:(id)sender;
- (IBAction)chooseNewRight:(id)sender;
- (IBAction)ignoreAllWhitespace:(id)sender;
- (IBAction)ignoreBlankLines:(id)sender;
- (IBAction)ignoreCase:(id)sender;
- (IBAction)ignoreWhitespaceChanges:(id)sender;
- (IBAction)reverseFiles:(id)sender;
- (IBAction)reloadDiff:(id)sender;

- (IBAction)nextLine:(id)sender;
- (IBAction)nextChange:(id)sender;
- (IBAction)prevLine:(id)sender;
- (IBAction)prevChange:(id)sender;

@end
