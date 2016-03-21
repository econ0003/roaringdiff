I have open sourced RoaringDiff.

This is all very preliminary.

There are a bunch of issues opened in the tracker of bugs I know of--please feel free to open new bugs.

If you are looking for binaries, go to:  http://www.biscade.com/tools/diff/

## Patches ##
I am happy to take patches.  Please follow the same code style that is in place now.

## Code Style ##
Roughly:

  * tabs are 4 spaces
  * single line/short comments are C++
  * function and structure comments are doxygen-style comments
  * Big multi-line comments are C with /**and**/ on separate lines
  * C function names are\_like\_this
  * Objective C functions are camelCase
  * use #if 0 to "comment out" big blocks of code
  * code wraps at roughly 100 columns
  * opening braces goes on the same line as the condition
    * for functions, opening brace goes on its own line
  * all if/while/for blocks get braces, even short ones
  * use 'XXX' to flag hacks or corner cases that need to be considered
  * no #pragmas except where absolutely necessary (gcc sprintf checks, packing structures, etc.)
  * #if header\_id / #define header\_id instead of #pragma once
  * no use of double underbars () on identifiers.

## Building ##
The code ([r8](https://code.google.com/p/roaringdiff/source/detail?r=8)) currently builds with about 26 warnings, mostly about id's and unknown messages.  I remember in 2006 I spent some time trying to track down how to name these variables properly without circular header issues.  If you have suggestions on how to resolve these, I'd be happy to hear it.

With pedantic warnings on, the code generates a bunch of warnings about #import.  I haven't figured out how to pass a flag to GCC from Xcode to ignore these warnings.

Long term, I'd like to get the code building with no warnings with pedantic turned on.

## Contact ##
mitch.haile@gmail.com