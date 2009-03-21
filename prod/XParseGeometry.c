/* 
 * This implementation of XParseGeometry comes with this license:
 *
 * Copyright (c) Mark J. Kilgard, 1994. 
 * 
 * This program is freely distributable without licensing fees
 * and is provided without guarantee or warrantee expressed or
 * implied. This program is -not- in the public domain.
 */

#include "XParseGeometry.h"
#include <unistd.h>

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */
static int
ReadInteger(char *s, char **NextString)
{
    register int Result = 0;
    int Sign = 1;
    
    if (*s == '+')
        s++;
    else if (*s == '-')
    {
        s++;
        Sign = -1;
    }
    
    for (; (*s >= '0') && (*s <= '9'); s++)
    {
        Result = (Result * 10) + (*s - '0');
    }
    
    *NextString = s;
    if (Sign >= 0)
        return (Result);
    else
        return (-Result);
}

int
My_XParseGeometry(char *s, my_geometry_t *geom)
{
    int *x = &geom->geom_xoffset;
    int *y = &geom->geom_yoffset;
    int *width  = &geom->geom_width;
    int *height = &geom->geom_height;
    
	int mask = NoValue;
	register char *strind;
	unsigned int tempWidth = 0, tempHeight = 0;
	int tempX = 0, tempY = 0;
	char *nextCharacter;

	if ( (s == NULL) || (*s == '\0')) return(mask);
	if (*s == '=')
		s++;  /* ignore possible '=' at beg of geometry spec */

	strind = (char *)s;
	if (*strind != '+' && *strind != '-' && *strind != 'x') {
		tempWidth = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter) 
		    return (0);
		strind = nextCharacter;
		mask |= WidthValue;
	}

	if (*strind == 'x') {	
		strind++;
		tempHeight = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= HeightValue;
	}

	if ((*strind == '+') || (*strind == '-')) {
		if (*strind == '-') {
  			strind++;
			tempX = -ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return (0);
			strind = nextCharacter;
			mask |= XNegative;

		}
		else
		{	strind++;
			tempX = ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return(0);
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-')) {
			if (*strind == '-') {
				strind++;
				tempY = -ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
				mask |= YNegative;

			}
			else
			{
				strind++;
				tempY = ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
			}
			mask |= YValue;
		}
	}
	
	/* If strind isn't at the end of the string the it's an invalid
		geometry specification. */

	if (*strind != '\0') return (0);

	if (mask & XValue)
	    *x = tempX;
 	if (mask & YValue)
	    *y = tempY;
	if (mask & WidthValue)
            *width = tempWidth;
	if (mask & HeightValue)
            *height = tempHeight;
	return (mask);
}
