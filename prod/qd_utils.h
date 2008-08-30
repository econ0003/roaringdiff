/*
 *  qd_utils.h
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

#ifndef _QD_UTILS_H
#define _QD_UTILS_H

#include <Cocoa/Cocoa.h>

void
qd_utils_rgbcolor_from_int(unsigned long c, RGBColor *col);

void
qd_utils_draw_text(short x, short y, const char *s, int tab_width, int truncate_at, int start_at, int slen);

unsigned long
qd_utils_darken_color(unsigned long c, int factor);

/**
 * XXX: Not really QuickDraw :-)
 */
NSColor *
qd_utils_nscolor_from_int(unsigned long c);

unsigned long
qd_utils_int_from_nscolor(NSColor *c);

#endif
