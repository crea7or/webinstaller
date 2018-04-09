// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "windows/windows_utils.h"
#include "installer/system_utils.h"

namespace WindowsUtils
{
  // RectHolder
  LONG RectHolder::Width()
  {
    return right - left;
  }

  LONG RectHolder::Height()
  {
    return bottom - top;
  }

  void RectHolder::AddOffset( LONG leftOffset, LONG topOffset )
  {
    right += leftOffset;
    left += leftOffset;
    bottom += topOffset;
    top += topOffset;
  }

  void RectHolder::Move( LONG newLeft, LONG newTop )
  {
    right = ( right - left + newLeft );
    left = newLeft;
    //bottom += ( newTop - top );
    bottom = ( bottom - top + newTop );
    top = newTop;
  }

  void RectHolder::Scale( LONG multiplicator, LONG denominator )
  {
    ScaleX( multiplicator, denominator );
    ScaleY( multiplicator, denominator );
  }

  void RectHolder::ScaleX( LONG multiplicator, LONG denominator )
  {
    left = MultiplyThenDivide( left, multiplicator, denominator );
    right = MultiplyThenDivide( right, multiplicator, denominator );
  }

  void RectHolder::ScaleY( LONG multiplicator, LONG denominator )
  {
    top = MultiplyThenDivide( top, multiplicator, denominator );
    bottom = MultiplyThenDivide( bottom, multiplicator, denominator );
  }
  // RectHolder

  HANDLE LoadFontFromResource( HINSTANCE instanceHandle, UINT resourceId )
  {
    HANDLE fontHandle = 0;
    HRSRC res = FindResource( instanceHandle, MAKEINTRESOURCE( resourceId ), L"BINARY" );
    if ( res )
    {
      HGLOBAL mem = LoadResource( instanceHandle, res );
      if ( mem != NULL )
      {
        void *data = LockResource( mem );
        size_t len = SizeofResource( instanceHandle, res );
        DWORD fontsInstalled = 0;
        fontHandle = AddFontMemResourceEx( data, len, NULL, &fontsInstalled );
      }
    }
    return fontHandle;
  }

  bool UnloadFontFromResource( HANDLE fontHandle )
  {
    bool result = false;
    if ( fontHandle != NULL )
    {
      result = RemoveFontMemResourceEx( fontHandle ) == TRUE;
    }
    return result;
  }

  void BlurBitmap( const HDC context, const HBITMAP bitmap, const DWORD width, const DWORD height, const DWORD passes )
  {
    DWORD bitmapSizeInBytes = width * 4 * height;
    BITMAPINFO bi;
    bi.bmiHeader.biSize = sizeof( bi.bmiHeader );
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = bitmapSizeInBytes;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;
    DWORD* sourceBuffer = ( DWORD* ) malloc( bitmapSizeInBytes );
    DWORD* destinationBuffer = ( DWORD* ) malloc( bitmapSizeInBytes );
    GetDIBits( context, bitmap, 0, height, ( LPVOID ) sourceBuffer, &bi, DIB_RGB_COLORS );
    memcpy( destinationBuffer, sourceBuffer, bitmapSizeInBytes );

    DWORD* srcBuffer = sourceBuffer;
    DWORD* dstBuffer = destinationBuffer;
    DWORD* tmpBuffer;

    DWORD c2, c3, c4, c5;
    DWORD r, g, b;
    DWORD pixelIndex;

    for ( DWORD steps = 0; steps < passes; ++steps )
    {
      for ( DWORD i = 1; i < ( height - 2 ); ++i )
      {
        pixelIndex = ( i * width );
        for ( DWORD j = 0; j < width; ++j )
        {
          c2 = GetBValue( srcBuffer[ pixelIndex - width ] );
          c3 = GetBValue( srcBuffer[ pixelIndex + 1 ] );
          c4 = GetBValue( srcBuffer[ pixelIndex + width ] );
          c5 = GetBValue( srcBuffer[ pixelIndex - 1 ] );
          r = ( GetBValue( srcBuffer[ pixelIndex ] ) + c2 + c3 + c4 + c5 ) / 5;

          c2 = GetGValue( srcBuffer[ pixelIndex - width ] );
          c3 = GetGValue( srcBuffer[ pixelIndex + 1 ] );
          c4 = GetGValue( srcBuffer[ pixelIndex + width ] );
          c5 = GetGValue( srcBuffer[ pixelIndex - 1 ] );
          g = ( GetGValue( srcBuffer[ pixelIndex ] ) + c2 + c3 + c4 + c5 ) / 5;

          c2 = GetRValue( srcBuffer[ pixelIndex - width ] );
          c3 = GetRValue( srcBuffer[ pixelIndex + 1 ] );
          c4 = GetRValue( srcBuffer[ pixelIndex + width ] );
          c5 = GetRValue( srcBuffer[ pixelIndex - 1 ] );
          b = ( GetRValue( srcBuffer[ pixelIndex ] ) + c2 + c3 + c4 + c5 ) / 5;
          // store in reverse order
          dstBuffer[ pixelIndex++ ] = RGB( b, g, r );
        }
      }
      tmpBuffer = srcBuffer;
      srcBuffer = dstBuffer;
      dstBuffer = tmpBuffer;
    }

    // Store tmpBuf
    SetDIBits( context, bitmap, 0, height, srcBuffer, &bi, DIB_RGB_COLORS );

    free(( LPVOID ) sourceBuffer );
    free(( LPVOID ) destinationBuffer );
  }

  UINT GetWindowDpi( HWND window )
  {
    DWORD version = GetVersion();
    UINT dpi = 0;
    DWORD majorVersion = ( DWORD ) ( LOBYTE( LOWORD( version )));
    DWORD minorVersion = ( DWORD ) ( HIBYTE( LOWORD( version )));

    if ( majorVersion >= 10 )
    {
      // Windows 10 and newer
      dpi = SystemUtils::GetDpiForWindowUser32( window );
    }
    else if ( majorVersion == 6 && minorVersion >= 3 )
    {
      // Windows 8.1
      dpi = SystemUtils::GetDpiForMonitorShcore( window );
    }
    if ( dpi == 0 )
    {
      // handle systems before Windows 10/8.1
      HDC desktopDC = GetDC( NULL );
      dpi = ( UINT ) GetDeviceCaps( desktopDC, LOGPIXELSY );
      ReleaseDC( NULL, desktopDC );
    }
    return dpi;
  }

  void FillLogFont( LOGFONT& fontStruct, const LONG height, const HWND window, const wchar_t* faceName, const bool forceAntiAlias )
  {
    wcscpy_s( fontStruct.lfFaceName, LF_FACESIZE, faceName == NULL ? L"webinstaller" : faceName );
    fontStruct.lfCharSet = ANSI_CHARSET;
    fontStruct.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    fontStruct.lfEscapement = fontStruct.lfOrientation = 0;
    fontStruct.lfHeight = 0 - MultiplyThenDivide( height, WindowsUtils::GetWindowDpi( window ), 72 );
    fontStruct.lfItalic = FALSE;
    fontStruct.lfOutPrecision = OUT_TT_PRECIS;
    fontStruct.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    if ( forceAntiAlias )
    {
      fontStruct.lfQuality = ANTIALIASED_QUALITY;
    }
    else
    {
      fontStruct.lfQuality = CLEARTYPE_QUALITY;
    }
    fontStruct.lfStrikeOut = FALSE;
    fontStruct.lfUnderline = FALSE;
    fontStruct.lfWidth = 0;
    fontStruct.lfWeight = FW_NORMAL;
  }

  LONG MultiplyThenDivide( LONG number, LONG multiplicator, LONG denominator )
  {
    int64_t multiply = ( int64_t ) number * ( int64_t ) multiplicator;
    if ( denominator != 0 )
    {
      return ( LONG ) ( multiply / denominator );
    }
    else
    {
      return -1;
    }
  }

  // find first DLGITEMTEMPLATEEX after DLGTEMPLATEEX
  /*
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
  */
  DLGITEMTEMPLATEEX* GetFirstItemTemplateEx( DLGTEMPLATEEX* dialogTemplate )
  {
    // add default size of DLGTEMPLATEEX
    LPBYTE itemTemplate = ( LPBYTE )dialogTemplate + sizeof( DLGTEMPLATEEX );

    // now it should be menu WORD array
    if ((( WORD* ) itemTemplate )[ 0 ] == 0 )
    {
      itemTemplate += sizeof( WORD );
    }
    else
    {
      itemTemplate += sizeof( WORD ) * 2;
    }

    // windowClass
    if ((( WORD* ) itemTemplate )[ 0 ] == 0 )
    {
      itemTemplate += sizeof( WORD );
    }
    else if ((( WORD* ) itemTemplate )[ 0 ] == 0xFFFF )
    {
      itemTemplate += sizeof( WORD ) * 2;
    }
    else
    {
      size_t stringSize = wcslen(( WCHAR* ) itemTemplate );
      // NULL termination char
      ++stringSize;
      itemTemplate += stringSize * sizeof( WCHAR );
    }

    // title
    if ((( WORD* ) itemTemplate )[ 0 ] == 0 )
    {
      itemTemplate += sizeof( WORD );
    }
    else
    {
      size_t stringSize = wcslen(( WCHAR* ) itemTemplate );
      // NULL termination char
      ++stringSize;
      itemTemplate += stringSize * sizeof( WCHAR );
    }

    if (( dialogTemplate->style & ( DS_SETFONT | DS_SHELLFONT )) > 0 )
    {
      // WORD pointsize;
      // WORD weight;
      // BYTE italic;
      // BYTE charset;
      itemTemplate += sizeof( WORD ) + sizeof( WORD ) + sizeof( BYTE ) + sizeof( BYTE );

      // there is font struct
      size_t stringSize = wcslen(( WCHAR* ) itemTemplate );
      // NULL termination char
      ++stringSize;
      itemTemplate += stringSize * sizeof( WCHAR );
    }
    // first DLGITEMTEMPLATEEX structure begins on the first DWORD boundary after the typeface string.
    // first DLGITEMTEMPLATEEX structure begins on the first DWORD boundary after the title string.

    size_t align = ( size_t )itemTemplate & ( sizeof( DWORD ) - 1 );
    if ( align > 0 )
    {
      itemTemplate += ( sizeof( DWORD ) - align );
    }
    return ( DLGITEMTEMPLATEEX* ) itemTemplate;
  }

  /*
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
    WORD extraCount; // The number of bytes of creation data after this member
  } DLGITEMTEMPLATEEX, *LPDLGITEMTEMPLATEEX;
  */

  // find next DLGITEMTEMPLATEEX after first DLGITEMTEMPLATEEX
  // function does not know if there any structs actually
  // use with caution and check cDlgItems item in DLGTEMPLATEEX
  DLGITEMTEMPLATEEX* GetNextItemTemplateEx( DLGITEMTEMPLATEEX* itemTemplate )
  {
    LPBYTE nextItemTemplate = ( LPBYTE )itemTemplate + sizeof( DLGITEMTEMPLATEEX );
    // windowClass
    if ((( WORD* ) nextItemTemplate )[ 0 ] == 0xFFFF )
    {
      nextItemTemplate += sizeof( WORD ) * 2;
    }
    else
    {
      size_t stringSize = wcslen(( WCHAR* ) nextItemTemplate );
      // NULL termination char
      ++stringSize;
      nextItemTemplate += stringSize * sizeof( WCHAR );
    }

    // title
    if ((( WORD* ) nextItemTemplate )[ 0 ] == 0xFFFF )
    {
      nextItemTemplate += sizeof( WORD ) * 2;
    }
    else
    {
      size_t stringSize = wcslen(( WCHAR* ) nextItemTemplate );
      // NULL termination char
      ++stringSize;
      nextItemTemplate += stringSize * sizeof( WCHAR );
    }

    if ((( WORD* ) nextItemTemplate )[ 0 ] == 0 )
    {
      nextItemTemplate += sizeof( WORD );
    }
    else
    {
      nextItemTemplate += (( WORD* ) nextItemTemplate )[ 0 ];
    }

    // next DLGITEMTEMPLATEEX structure begins on the first DWORD boundary after this
    size_t align = ( size_t ) nextItemTemplate & ( sizeof( DWORD ) - 1 );
    if ( align > 0 )
    {
      nextItemTemplate += ( sizeof( DWORD ) - align );
    }
    return ( DLGITEMTEMPLATEEX*)nextItemTemplate;
  }
}
