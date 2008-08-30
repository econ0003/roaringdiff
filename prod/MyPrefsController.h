/*
 *  MyPrefsController.h
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

@interface MyPrefsController : NSObject
{
    IBOutlet id fontSize;
    IBOutlet id showLineNos;
    IBOutlet id tabWidth;
	
	IBOutlet id ignoreCase;
	IBOutlet id ignoreWhitespaceChanges;
	IBOutlet id ignoreAllWhitespace;
	IBOutlet id ignoreBlankLines;
	
    IBOutlet id colorAdd;
    IBOutlet id colorDel;
    IBOutlet id colorChange;
    IBOutlet id colorBackground;
	IBOutlet id diffWindow;
}
- (IBAction)editIgnoreCase:(id)sender;
- (IBAction)editIgnoreWhitespaceChanges:(id)sender;
- (IBAction)editIgnoreAllWhitespace:(id)sender;
- (IBAction)editIgnoreBlankLines:(id)sender;
- (IBAction)editColorAdd:(id)sender;
- (IBAction)editColorBackground:(id)sender;
- (IBAction)editColorChange:(id)sender;
- (IBAction)editColorDel:(id)sender;
- (IBAction)editFontSize:(id)sender;
- (IBAction)editTabWidth:(id)sender;
- (IBAction)toggleShowLineNos:(id)sender;
- (void)awakeFromNib;
@end
