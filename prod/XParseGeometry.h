/* 
 * This implementation of XParseGeometry comes with this license:
 *
 * Copyright (c) Mark J. Kilgard, 1994. 
 * 
 * This program is freely distributable without licensing fees
 * and is provided without guarantee or warrantee expressed or
 * implied. This program is -not- in the public domain.
 */

#ifndef _ROARINGDIFF_X_PARSE_GEOMETRY_H
#define _ROARINGDIFF_X_PARSE_GEOMETRY_H

enum {
    NoValue = (1 << 0),
    WidthValue = (1 << 1),
    HeightValue = (1 << 2),
    XNegative = (1 << 3),
    XValue = (1 << 4),
    YNegative = (1 << 5),
    YValue = (1 << 6)
};

typedef struct {
    int geom_width;
    int geom_height;
    int geom_xoffset;
    int geom_yoffset;
} my_geometry_t;

int
My_XParseGeometry(char *s, my_geometry_t *geom);

#endif