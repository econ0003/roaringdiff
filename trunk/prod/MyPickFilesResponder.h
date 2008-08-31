/*
 *  MyPickFilesResponder.h
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

#include "MyDiffMenuController.h"

@interface MyPickFilesResponder : NSObject
{
    IBOutlet id leftFileStats;
    IBOutlet id rightFileStats;
    IBOutlet id leftFileName;
    IBOutlet id rightFileName;
    IBOutlet id diffWindow;
    IBOutlet id diffView;
    IBOutlet id changeMenu;
    IBOutlet MyDiffMenuController *changeMenuController;
}
- (void)awakeFromNib;
- (IBAction)buttonCompareFiles:(id)sender;
- (IBAction)compareFiles:(id)sender leftLabel:(char*)leftLabel rightLabel:(char*)rightLabel;
- (IBAction)leftFileSelect:(id)sender;
- (IBAction)leftFileTextEntry:(id)sender;
- (IBAction)rightFileSelect:(id)sender;
- (IBAction)rightFileTextEntry:(id)sender;

- (int)numberOfItemsInComboBox:(NSComboBox *)aComboBox;
- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(int)index;

- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo;

@end
