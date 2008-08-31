/*
 *  MyHorzCompareView.m
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

#import "MyHorzCompareView.h"
#include "rd_prefs.h"
#include "qd_utils.h"

#define LEFT_MARGIN 40
#define LEFT_TEXT (LEFT_MARGIN + 8)

#define LINE_1_TOP 15
#define HORZ_LINE_Y 22
#define LINE_2_TOP 35

static void
hc_draw_decoration(NSRect rect)
{
    RGBColor dark_gray = { 0x6666, 0x6666, 0x6666 };
    Rect qdr;
    qdr.left   = (short) 0;
    qdr.top    = (short) 0;
    qdr.right  = (short) rect.size.width;
    qdr.bottom = (short) rect.size.height;
    
    RGBColor col;
    qd_utils_rgbcolor_from_int(rd_prefs_get_color_background(), &col);
    RGBForeColor(&col);
    PaintRect(&qdr);
    
    RGBForeColor(&dark_gray);
    qdr.bottom--;
    qdr.right--;
    FrameRect(&qdr);

    // put in a horz line between the two segments
    MoveTo(0,         HORZ_LINE_Y);
    LineTo(qdr.right, HORZ_LINE_Y);

    // draw a vertical line for the left margin
    qdr.left = LEFT_MARGIN - 2;
    qdr.right = qdr.left + 2;
    PaintRect(&qdr);
    
    // label the two lines
    const char *msg = NULL;
    int w = 0;
    int x = 0;
    
    msg = "left";
    w = TextWidth(msg, 0, strlen(msg));
    x = LEFT_MARGIN - 4 - w;
    MoveTo(x, LINE_1_TOP);
    DrawText(msg, 0, strlen(msg));

    msg = "right";
    w = TextWidth(msg, 0, strlen(msg));
    x = LEFT_MARGIN - 4 - w;
    MoveTo(x, LINE_2_TOP);
    DrawText(msg, 0, strlen(msg));
}

@implementation MyHorzCompareView

- (void)drawRect:(NSRect)rect
{
    short font_id = 0;
    GetFNum("\pCourier", &font_id);
    TextFont(font_id);
    TextSize(10);
    
    hc_draw_decoration(rect);
    
    int line = [diffView selectedLine];
    if (line < 0) {
        const char *msg = "(No line selected)";
        ForeColor(blackColor);
        qd_utils_draw_text(LEFT_TEXT, LINE_1_TOP, msg, 4, -1, 0, strlen(msg));
    } else {
        const char *ls = [diffView selectedLeftLine];
        const char *rs = [diffView selectedRightLine];
        if (ls == NULL) {
            ls = "";
        }
        
        if (rs == NULL) {
            rs = "";
        }
        
        ForeColor(blackColor);
        qd_utils_draw_text(LEFT_TEXT, LINE_1_TOP, ls, 4, -1, 0, strlen(ls));
        qd_utils_draw_text(LEFT_TEXT, LINE_2_TOP, rs, 4, -1, 0, strlen(rs));
    }
}

@end
