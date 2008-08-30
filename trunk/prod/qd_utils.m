/*
 *  qd_utils.c
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

#include "qd_utils.h"
#include "diff.h"

void
qd_utils_rgbcolor_from_int(unsigned long c, RGBColor *col)
{
	assert(col != NULL);
	short r = 0;
	short g = 0;
	short b = 0;
	
	r = ((c & 0x00ff0000) >> 16);
	r = r | (r << 8);
	
	g = ((c & 0x0000ff00) >> 8);
	g = g | (g << 8);

	b = (c & 0x000000ff);
	b = b | (b << 8);

	col->red = r;
	col->green = g;
	col->blue = b;
}

unsigned long
qd_utils_darken_color(unsigned long c, int factor)
{
	int r = 0;
	int g = 0;
	int b = 0;
	
	r = ((c & 0x00ff0000) >> 16);
	g = ((c & 0x0000ff00) >> 8);
	b = (c & 0x000000ff);

	r = (r * factor) & 0xff;
	g = (g * factor) & 0xff;
	b = (b * factor) & 0xff;

	return ((r << 16) | (g << 8) | (b));
}

NSColor *
qd_utils_nscolor_from_int(unsigned long c)
{
	float r = ((float)((c & 0x00ff0000) >> 16)) / 255.0f;
	float g = ((float)((c & 0x0000ff00) >>  8)) / 255.0f;
	float b = ((float) (c & 0x000000ff))        / 255.0f;
	
	return [NSColor colorWithCalibratedRed:r green:g blue:b alpha:1.0f];
}

unsigned long
qd_utils_int_from_nscolor(NSColor *c)
{
	float r = [c redComponent];
	float g = [c greenComponent];
	float b = [c blueComponent];
	
	int ri = 255 * r;
	int gi = 255 * g;
	int bi = 255 * b;
	
	return (ri << 16) | (gi << 8) | (bi);
}

void
qd_utils_draw_text(short x, short y, const char *s, int tab_width, int truncate_at, int start_at, int slen)
{
	assert(s != NULL);
	
	MoveTo(x, y);
	static char t[DIFF_MAX_LINE_LENGTH]; /* XXX: maybe not long enough */
	bzero(t, sizeof(t));
	int i, j;
	for (i = 0, j = 0; s[i] != '\0' && j < sizeof(t) - tab_width; i++, j++) {
		if (s[i] == '\t') {
			t[j++] = ' ';
			while ((j % tab_width) != 0) {		/* align on a <tab_width> colum boundary. */
				t[j++] = ' ';
				slen++;  	/* We're adding to the string length here */
			}
			
			j--;
		} else {
			t[j] = s[i];
		}
	}
	
	if (truncate_at > 0) {
		assert(truncate_at + 5 < sizeof(t));
		if (t[truncate_at] != '\0') {
			t[truncate_at + 0] = '.';
			t[truncate_at + 1] = '.';
			t[truncate_at + 2] = '.';
			bzero(&t[truncate_at + 3], sizeof(t) - truncate_at - 3);
		}
	}
	
	slen -= start_at;
	DrawText(t, start_at, slen);
}
