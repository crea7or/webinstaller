// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <string>

// these structs are not defined in any header in windows sdk
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms645398(v=vs.85).aspx
#pragma pack(push, 1)
typedef struct
{
  WORD dlgVer;
  WORD signature;
  DWORD helpID;
  DWORD exStyle;
  DWORD style;
  WORD cDlgItems;
  short x;
  short y;
  short cx;
  short cy;
  // WORD menu[]; if [0] == 0x0000 = no other elements, if [0] == 0xFFFF = one additional element
  // WORD windowClass[]; if [0] == 0x0000 = no other elements, if [0] == 0xFFFF = one additional element, else LPWSTR
  // WCHAR title[]; if [0] == 0x0000 = no other elements, else LPWSTR
  //
  // other members is present only if the style member specifies DS_SETFONT or DS_SHELLFONT.
  // WORD pointsize;
  // WORD weight;
  // BYTE italic;
  // BYTE charset;
  // WCHAR typeface[];
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;

// Each DLGITEMTEMPLATEEX structure in the template must be aligned on a DWORD boundary.
// If the style member specifies the DS_SETFONT or DS_SHELLFONT style, the first DLGITEMTEMPLATEEX structure begins on the first DWORD boundary after the typeface string.
// If these styles are not specified, the first structure begins on the first DWORD boundary after the title string.
//
// The menu, windowClass, title, and typeface arrays must be aligned on WORD boundaries.

typedef struct
{
  DWORD helpID;
  DWORD exStyle;
  DWORD style;
  short x;
  short y;
  short cx;
  short cy;
  DWORD id;
  // WORD windowClass[]; if [0] == 0xFFFF - one additional element, else LPWSTR
  // WORD title[]; if [0] == 0xFFFF - one additional element, else LPWSTR
  // WORD extraCount; The number of bytes of creation data after this member
} DLGITEMTEMPLATEEX, *LPDLGITEMTEMPLATEEX;
#pragma pack(pop)

namespace WindowsUtils
{
  // RectHolder
  struct RectHolder : public RECT
  {
    LONG Width();
    LONG Height();
    void AddOffset( LONG leftOffset, LONG topOffset );
    void Move( LONG newLeft, LONG newTop );
    void Scale( LONG multiplicator, LONG denominator );
    void ScaleX( LONG multiplicator, LONG denominator );
    void ScaleY( LONG multiplicator, LONG denominator );
  };
  // RectHolder

  // MulDiv replacement (no rounding)
  LONG MultiplyThenDivide( LONG number, LONG multiplicator, LONG denominator );

  // load custom font resource
  HANDLE LoadFontFromResource( HINSTANCE instanceHandle, UINT resourceId );

  // unload custom font resource
  bool UnloadFontFromResource( HANDLE fontHandle );

  // blur bitmap with bicubic algo
  void BlurBitmap( const HDC context, const HBITMAP bitmap, const DWORD width, const DWORD height, const DWORD passes );

  // HiDpi functions
  UINT GetWindowDpi( HWND window );

  // fill LOGFONT structure with our font
  void FillLogFont( LOGFONT& fontStruct, const LONG height, const HWND window = NULL, const wchar_t* faceName = NULL, const bool forceAntiAlias = false );

  // find first DLGITEMTEMPLATEEX after DLGTEMPLATEEX
  DLGITEMTEMPLATEEX* GetFirstItemTemplateEx( DLGTEMPLATEEX* dialogTemplate );

  // find next DLGITEMTEMPLATEEX after first DLGITEMTEMPLATEEX
  DLGITEMTEMPLATEEX* GetNextItemTemplateEx( DLGITEMTEMPLATEEX* itemTemplate );

};

