/*
   Copyright (C) 2000 Nate Miller nkmiller@calpoly.edu

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   See gpl.txt for more information regarding the GNU General Public License.
*/

/*
   To use this mouse class with GLUT make sure that you #define __GLUTMOUSE__
before you #include "mouse.h".  I was thinking about having this header check
for GLUT #defines, but you would have had to #include <glut.h> before this
header which could lead to problems.
*/

#ifndef __MOUSEH__
#define __MOUSEH__

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __GLUTMOUSE__
#include <GL/glut.h>
#endif

enum
{
   buttonLeft = 1,
   buttonRight = 2,
   buttonMiddle = 4,
};

struct mouse_t
{
   mouse_t() : oldX(-1), oldY(-1), currX(-1), currY(-1) {Reset();}
   inline void SetState(int btn, int ste = 0);
   inline void SetNewPos(int x, int y);
   int IsLeft(void) const           {return (state & buttonLeft);}
   int IsRight(void) const          {return (state & buttonRight);}
   int IsMiddle(void) const         {return (state & buttonMiddle);}
   void Reset(void)                 {state = 0;}
   int GetOldX(void) const          {return oldX;}
   int GetOldY(void) const          {return oldY;}
   int GetX(void) const             {return currX;}
   int GetY(void) const             {return currY;}
   int GetDiffX(int x) const        {return x - oldX;}
   int GetDiffY(int y) const        {return y - oldY;}
   int GetDiffX(void) const         {return currX - oldX;}
   int GetDiffY(void) const         {return currY - oldY;}
   void SetOldPos(int x, int y)     {oldX = x; oldY = y;}
#ifdef _WIN32
   void SetCapture(HWND hWnd) const    {SetCapture(hWnd);}
   void Release(void) const         {ReleaseCapture();}
#endif

protected:
   int state;
   int oldX;
   int oldY;
   int currX;
   int currY;

};

inline void mouse_t::SetState(int btn, int ste)
{

#ifdef _WIN32
#ifndef __GLUTMOUSE__
   switch (btn)
   {
      case WM_LBUTTONDOWN:
         state |= buttonLeft;
      break;
      case WM_RBUTTONDOWN:
         state |= buttonRight;
      break;
      case WM_MBUTTONDOWN:
         state |= buttonMiddle;
      break;
      case WM_LBUTTONUP:
         state &= ~buttonLeft;
      break;
      case WM_RBUTTONUP:
         state &= ~buttonRight;
      break;
      case WM_MBUTTONUP:
         state &= ~buttonMiddle;
      break;
   }
#endif
#endif

#ifdef __GLUTMOUSE__

   if (ste == GLUT_DOWN)
   {
      switch(btn)
      {
         case GLUT_LEFT_BUTTON:
            state |= buttonLeft;
         break;
         case GLUT_RIGHT_BUTTON:
            state |= buttonRight;
         break;
         case GLUT_MIDDLE_BUTTON:
            state |= buttonMiddle;
         break;
      }
   }
   else if (ste == GLUT_UP)
   {
      switch(btn)
      {
         case GLUT_LEFT_BUTTON:
            state &= ~buttonLeft;
         break;
         case GLUT_RIGHT_BUTTON:
            state &= ~buttonRight;
         break;
         case GLUT_MIDDLE_BUTTON:
            state &= ~buttonMiddle;
         break;
      }
   }
#endif
}

inline void mouse_t::SetNewPos(int x, int y)
{
   currX = x;
   currY = y;
#ifdef _WIN32
#ifndef __GLUTMOUSE__
   if(currX & 1 << 15)
      currX -= (1 << 16);

	if(currY & 1 << 15)
      currY -= (1 << 16);
#endif
#endif
}

#endif
