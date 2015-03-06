// uglyfont.cpp
//
// Copyright: Soji Yamakawa (CaptainYS, E-Mail: PEB01130*nifty+com  <- Replace * with @, + with .)
//
// I don't abandon the copyright, but you can use this code and the header 
// (uglyfont.cpp and uglyfont.h) for your product regardless of the purpose,
// i.e., free or commercial, open source or proprietary.
//
// However, I do not take any responsibility for the consequence of using
// this code and header.  Please use on your own risks.
//
// January 5, 2005
//
// Soji Yamakawa


#ifndef UGLYFONT_IS_INCLUDED
#define UGLYFONT_IS_INCLUDED
/* { */



// YsDrawUglyFont function draws text using an ugly vector-font set.
// The size of a letter are 1.0x1.0 (no thickness in z direction).
// The size and the location on the display can be controlled by 
// glScale and glTranslate (not glRasterPos)

// This function uses OpenGL's display list 1400 to 1655.  If it conflicts with
// your program, modify:
//    const int YsUglyFontBase=1400;
// in uglyfont.cpp

namespace uglyfont {

enum Justify {
  // horizontal justification
  LEFT=0x01,	// default
  CENTER=0x02,
  RIGHT=0x04,
  // vertical justification
  ABOVE=0x08,	// default
  MIDDLE=0x10,
  BELOW=0x20,
};

void drawFont(const char str[],double scale=0.1,int justify=0,int useDisplayList=1);

}

/* } */
#endif
