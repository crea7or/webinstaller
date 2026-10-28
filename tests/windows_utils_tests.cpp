// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "tests/test_framework.h"
#include "windows/windows_utils.h"
#include "installer/resource.h"

using WindowsUtils::RectHolder;

namespace
{
  RectHolder MakeRect( LONG left, LONG top, LONG right, LONG bottom )
  {
    RectHolder rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
  }

  bool SameRect( const RECT& a, LONG left, LONG top, LONG right, LONG bottom )
  {
    return a.left == left && a.top == top && a.right == right && a.bottom == bottom;
  }
}

// RectHolder

TEST( RectHolder, WidthAndHeight )
{
  RectHolder rect = MakeRect( 10, 20, 110, 70 );
  EXPECT_EQ( rect.Width(), 100 );
  EXPECT_EQ( rect.Height(), 50 );
}

TEST( RectHolder, AddOffsetMovesAllEdges )
{
  RectHolder rect = MakeRect( 10, 20, 110, 70 );
  rect.AddOffset( 5, -10 );
  EXPECT_TRUE( SameRect( rect, 15, 10, 115, 60 ));
}

TEST( RectHolder, MoveKeepsSize )
{
  RectHolder rect = MakeRect( 10, 20, 110, 70 );
  rect.Move( -3, 200 );
  EXPECT_TRUE( SameRect( rect, -3, 200, 97, 250 ));
}

TEST( RectHolder, ScaleBothAxes )
{
  RectHolder rect = MakeRect( 10, 20, 110, 70 );
  rect.Scale( 144, 96 );
  EXPECT_TRUE( SameRect( rect, 15, 30, 165, 105 ));
}

TEST( RectHolder, ScaleXOnlyChangesHorizontalEdges )
{
  RectHolder rect = MakeRect( 3, 3, 5, 5 );
  rect.ScaleX( 3, 2 );
  // truncated, not rounded
  EXPECT_TRUE( SameRect( rect, 4, 3, 7, 5 ));
}

TEST( RectHolder, ScaleYOnlyChangesVerticalEdges )
{
  RectHolder rect = MakeRect( 3, 8, 5, 16 );
  rect.ScaleY( 13, 8 );
  EXPECT_TRUE( SameRect( rect, 3, 13, 5, 26 ));
}

// MultiplyThenDivide

TEST( MultiplyThenDivide, BasicValues )
{
  EXPECT_EQ( WindowsUtils::MultiplyThenDivide( 10, 144, 96 ), 15 );
  EXPECT_EQ( WindowsUtils::MultiplyThenDivide( 0, 144, 96 ), 0 );
}

TEST( MultiplyThenDivide, TruncatesTowardZero )
{
  EXPECT_EQ( WindowsUtils::MultiplyThenDivide( 7, 1, 2 ), 3 );
  EXPECT_EQ( WindowsUtils::MultiplyThenDivide( -7, 1, 2 ), -3 );
}

TEST( MultiplyThenDivide, DoesNotOverflowIntermediateProduct )
{
  EXPECT_EQ( WindowsUtils::MultiplyThenDivide( 2000000000, 3, 3 ), 2000000000 );
}

TEST( MultiplyThenDivide, ZeroDenominatorReturnsMinusOne )
{
  EXPECT_EQ( WindowsUtils::MultiplyThenDivide( 10, 2, 0 ), -1 );
}

// GetWindowDpi / FillLogFont

TEST( GetWindowDpi, ReturnsPositiveDpiWithAndWithoutWindow )
{
  EXPECT_TRUE( WindowsUtils::GetWindowDpi( NULL ) > 0 );
  EXPECT_TRUE( WindowsUtils::GetWindowDpi( GetDesktopWindow()) > 0 );
}

TEST( FillLogFont, DefaultsToInstallerFace )
{
  LOGFONT font;
  WindowsUtils::FillLogFont( font, 10 );
  EXPECT_EQ( std::wstring( font.lfFaceName ), std::wstring( L"webinstaller" ));
  EXPECT_EQ( font.lfWeight, FW_NORMAL );
  EXPECT_EQ( font.lfItalic, FALSE );
  EXPECT_EQ( font.lfUnderline, FALSE );
  EXPECT_EQ( font.lfStrikeOut, FALSE );
  EXPECT_EQ( font.lfQuality, CLEARTYPE_QUALITY );
}

TEST( FillLogFont, UsesGivenFaceName )
{
  LOGFONT font;
  WindowsUtils::FillLogFont( font, 10, NULL, L"arial" );
  EXPECT_EQ( std::wstring( font.lfFaceName ), std::wstring( L"arial" ));
}

TEST( FillLogFont, HeightIsNegativePointsScaledByDpi )
{
  LOGFONT font;
  WindowsUtils::FillLogFont( font, 12 );
  LONG dpi = ( LONG ) WindowsUtils::GetWindowDpi( NULL );
  // negative height selects by character height
  EXPECT_EQ( font.lfHeight, -WindowsUtils::MultiplyThenDivide( 12, dpi, 72 ));
}

TEST( FillLogFont, ForceAntiAliasSelectsAntialiasedQuality )
{
  LOGFONT font;
  WindowsUtils::FillLogFont( font, 10, NULL, NULL, true );
  EXPECT_EQ( font.lfQuality, ANTIALIASED_QUALITY );
}

TEST( LoadFontFromResource, InstallerFontIsAvailableAfterLoading )
{
  // font is loaded by test main the same way the installer does
  LOGFONT logFont;
  WindowsUtils::FillLogFont( logFont, 10 );
  HFONT font = CreateFontIndirect( &logFont );
  HDC context = CreateCompatibleDC( NULL );
  HFONT previousFont = ( HFONT ) SelectObject( context, font );
  wchar_t faceName[ LF_FACESIZE ] = { 0 };
  GetTextFace( context, LF_FACESIZE, faceName );
  SelectObject( context, previousFont );
  DeleteDC( context );
  DeleteObject( font );
  // without the private font GDI substitutes another face
  EXPECT_EQ( std::wstring( faceName ), std::wstring( L"webinstaller" ));
}

TEST( LoadFontFromResource, MissingResourceReturnsNull )
{
  EXPECT_TRUE( WindowsUtils::LoadFontFromResource( GetModuleHandle( NULL ), 0xFFF0 ) == NULL );
  EXPECT_FALSE( WindowsUtils::UnloadFontFromResource( NULL ));
}

// dialog template walking

namespace
{
  // writes DLGTEMPLATEEX / DLGITEMTEMPLATEEX binary data
  class TemplateWriter
  {
  public:
    void Word( WORD value ) { Append( &value, sizeof( value )); }
    void Dword( DWORD value ) { Append( &value, sizeof( value )); }
    void Short( short value ) { Append( &value, sizeof( value )); }
    void Byte( BYTE value ) { Append( &value, sizeof( value )); }
    void String( const wchar_t* text ) { Append( text, ( wcslen( text ) + 1 ) * sizeof( wchar_t )); }
    void Ordinal( WORD value ) { Word( 0xFFFF ); Word( value ); }
    void AlignDword()
    {
      while ( bytes.size() % sizeof( DWORD ) != 0 )
      {
        bytes.push_back( 0 );
      }
    }

    void Header( DWORD style, WORD items )
    {
      Word( 1 );
      Word( 0xFFFF );
      Dword( 0 );
      Dword( 0 );
      Dword( style );
      Word( items );
      Short( 0 );
      Short( 0 );
      Short( 200 );
      Short( 100 );
    }

    // item header up to and including id
    void ItemStart( DWORD id, short x )
    {
      AlignDword();
      Dword( 0 );
      Dword( 0 );
      Dword( WS_CHILD | WS_VISIBLE );
      Short( x );
      Short( 1 );
      Short( 10 );
      Short( 10 );
      Dword( id );
    }

    // data is copied into a DWORD aligned buffer, like resources are
    DLGTEMPLATEEX* Data()
    {
      aligned.assign(( bytes.size() + 3 ) / 4, 0 );
      memcpy( aligned.data(), bytes.data(), bytes.size());
      return ( DLGTEMPLATEEX* ) aligned.data();
    }

  private:
    void Append( const void* data, size_t size )
    {
      const BYTE* begin = ( const BYTE* ) data;
      bytes.insert( bytes.end(), begin, begin + size );
    }

    std::vector< BYTE > bytes;
    std::vector< DWORD > aligned;
  };

  std::vector< DWORD > ItemIds( DLGTEMPLATEEX* dialog, std::vector< short >* xs = NULL )
  {
    std::vector< DWORD > ids;
    DLGITEMTEMPLATEEX* item = WindowsUtils::GetFirstItemTemplateEx( dialog );
    for ( WORD index = 0; index < dialog->cDlgItems; ++index )
    {
      ids.push_back( item->id );
      if ( xs != NULL )
      {
        xs->push_back( item->x );
      }
      item = WindowsUtils::GetNextItemTemplateEx( item );
    }
    return ids;
  }
}

TEST( DialogTemplate, OrdinalMenuAndClassWithoutFont )
{
  TemplateWriter writer;
  writer.Header( WS_POPUP, 2 );
  writer.Ordinal( 7 );     // menu
  writer.Ordinal( 0x8002 ); // class
  writer.String( L"T" );   // title
  writer.ItemStart( 11, 5 );
  writer.Ordinal( 0x0082 );
  writer.String( L"first" );
  writer.Word( 0 );
  writer.ItemStart( 22, 6 );
  writer.Ordinal( 0x0080 );
  writer.Ordinal( 3 );
  writer.Word( 0 );

  std::vector< short > xs;
  std::vector< DWORD > ids = ItemIds( writer.Data(), &xs );
  EXPECT_EQ( ids.size(), 2u );
  EXPECT_TRUE( ids == std::vector< DWORD >( { 11, 22 } ));
  EXPECT_TRUE( xs == std::vector< short >( { 5, 6 } ));
}

TEST( DialogTemplate, NamedMenuClassStringsAndFont )
{
  TemplateWriter writer;
  writer.Header( WS_POPUP | DS_SETFONT, 1 );
  writer.String( L"MAINMENU" );     // menu by name
  writer.String( L"CustomDialog" ); // class by name
  writer.String( L"Some title" );
  writer.Word( 8 );                 // point size
  writer.Word( FW_NORMAL );
  writer.Byte( 0 );                 // italic
  writer.Byte( DEFAULT_CHARSET );
  writer.String( L"MS Shell Dlg" );
  writer.ItemStart( 1234, 42 );
  writer.String( L"EDIT" );
  writer.String( L"" );
  writer.Word( 0 );

  std::vector< short > xs;
  std::vector< DWORD > ids = ItemIds( writer.Data(), &xs );
  EXPECT_EQ( ids.size(), 1u );
  EXPECT_TRUE( ids == std::vector< DWORD >( { 1234 } ));
  EXPECT_TRUE( xs == std::vector< short >( { 42 } ));
}

TEST( DialogTemplate, SkipsItemCreationData )
{
  TemplateWriter writer;
  writer.Header( WS_POPUP, 3 );
  writer.Word( 0 ); // no menu
  writer.Word( 0 ); // default class
  writer.Word( 0 ); // no title
  writer.ItemStart( 1, 1 );
  writer.Ordinal( 0x0082 );
  writer.String( L"a" );
  // 6 bytes of creation data after extraCount
  writer.Word( 6 );
  writer.Dword( 0xDEADBEEF );
  writer.Word( 0xFFFF );
  writer.ItemStart( 2, 2 );
  writer.Ordinal( 0x0082 );
  writer.String( L"bb" );
  writer.Word( 1 );
  writer.Byte( 0x55 );
  writer.ItemStart( 3, 3 );
  writer.Ordinal( 0x0080 );
  writer.String( L"ccc" );
  writer.Word( 0 );

  std::vector< DWORD > ids = ItemIds( writer.Data());
  EXPECT_TRUE( ids == std::vector< DWORD >( { 1, 2, 3 } ));
}

TEST( DialogTemplate, WalksInstallerMainDialog )
{
  HINSTANCE module = GetModuleHandle( NULL );
  HRSRC resource = FindResource( module, MAKEINTRESOURCE( IDD_DIALOG_MAIN ), RT_DIALOG );
  ASSERT_TRUE( resource != NULL );
  DLGTEMPLATEEX* dialog = ( DLGTEMPLATEEX* ) LockResource( LoadResource( module, resource ));
  ASSERT_TRUE( dialog != NULL );
  EXPECT_EQ( dialog->signature, 0xFFFF );
  EXPECT_EQ( dialog->cx, 450 );
  EXPECT_EQ( dialog->cy, 264 );

  std::vector< short > xs;
  std::vector< DWORD > ids = ItemIds( dialog, &xs );
  std::vector< DWORD > expectedIds( {
    IDC_STATIC_LOGO, IDC_STATIC_TEXT, IDC_STATIC_CLOSE, IDC_SETUP_BUTTON, IDC_STATIC_CHECKBOX_PLACEHOLDER,
    IDC_PROGRESS, IDC_STATIC_PROGRESS_TEXT, IDC_STATIC_MAIN_LICENSE, IDC_STATIC_DESCRIPTION } );
  EXPECT_TRUE( ids == expectedIds );
  EXPECT_TRUE( xs == std::vector< short >( { 176, 44, 424, 158, 45, 24, 28, 8, 11 } ));
}

// BlurBitmap

namespace
{
  // 32 bit top-down DIB section with direct pixel access
  class TestBitmap
  {
  public:
    TestBitmap( LONG _width, LONG _height, DWORD fill ) : width( _width ), height( _height ), pixels( NULL )
    {
      BITMAPINFO info = { 0 };
      info.bmiHeader.biSize = sizeof( info.bmiHeader );
      info.bmiHeader.biWidth = width;
      // BlurBitmap reads bottom-up rows, symmetric kernel makes orientation irrelevant
      info.bmiHeader.biHeight = height;
      info.bmiHeader.biPlanes = 1;
      info.bmiHeader.biBitCount = 32;
      info.bmiHeader.biCompression = BI_RGB;
      bitmap = CreateDIBSection( NULL, &info, DIB_RGB_COLORS, ( void** ) &pixels, NULL, 0 );
      for ( LONG index = 0; index < width * height; ++index )
      {
        pixels[ index ] = fill;
      }
    }

    ~TestBitmap()
    {
      DeleteObject( bitmap );
    }

    DWORD& At( LONG x, LONG y )
    {
      return pixels[ y * width + x ];
    }

    void Blur( DWORD passes )
    {
      HDC screen = GetDC( NULL );
      WindowsUtils::BlurBitmap( screen, bitmap, width, height, passes );
      ReleaseDC( NULL, screen );
      GdiFlush();
    }

    LONG width;
    LONG height;
    DWORD* pixels;
    HBITMAP bitmap;
  };
}

TEST( BlurBitmap, UniformImageIsUnchanged )
{
  TestBitmap image( 8, 6, 0x00336699 );
  image.Blur( 5 );
  bool allSame = true;
  for ( LONG index = 0; index < 8 * 6; ++index )
  {
    allSame = allSame && image.pixels[ index ] == 0x00336699;
  }
  EXPECT_TRUE( allSame );
}

TEST( BlurBitmap, OnePassAveragesCrossNeighbourhoodPerChannel )
{
  TestBitmap image( 5, 5, 0 );
  image.At( 2, 2 ) = 0x00FF7F0A;
  image.Blur( 1 );
  // center and its 4 neighbours get a fifth of each channel
  DWORD fifth = (( 0xFF / 5 ) << 16 ) | (( 0x7F / 5 ) << 8 ) | ( 0x0A / 5 );
  EXPECT_EQ( image.At( 2, 2 ), fifth );
  EXPECT_EQ( image.At( 1, 2 ), fifth );
  EXPECT_EQ( image.At( 3, 2 ), fifth );
  EXPECT_EQ( image.At( 2, 1 ), fifth );
  EXPECT_EQ( image.At( 2, 3 ), fifth );
  // diagonal neighbours are not in the kernel
  EXPECT_EQ( image.At( 1, 1 ), 0u );
  EXPECT_EQ( image.At( 3, 3 ), 0u );
}

TEST( BlurBitmap, BorderPixelsAreNotModified )
{
  TestBitmap image( 6, 6, 0 );
  // bright pixels next to the border must not leak into the border
  image.At( 1, 1 ) = 0x00FFFFFF;
  image.At( 4, 4 ) = 0x00FFFFFF;
  image.Blur( 3 );
  bool bordersBlack = true;
  for ( LONG index = 0; index < 6; ++index )
  {
    bordersBlack = bordersBlack && image.At( index, 0 ) == 0 && image.At( index, 5 ) == 0;
    bordersBlack = bordersBlack && image.At( 0, index ) == 0 && image.At( 5, index ) == 0;
  }
  EXPECT_TRUE( bordersBlack );
  EXPECT_NE( image.At( 2, 2 ), 0u );
}

TEST( BlurBitmap, RowEdgesDoNotWrapToNeighbourRows )
{
  TestBitmap image( 5, 5, 0 );
  // last pixel of row 1 is followed in memory by the first pixel of row 2,
  // they are not neighbours and must not be averaged together
  image.At( 4, 1 ) = 0x00FFFFFF;
  image.Blur( 1 );
  EXPECT_EQ( image.At( 0, 2 ), 0u );
  // but its real neighbour inside the row is blurred
  EXPECT_NE( image.At( 3, 1 ), 0u );
}

TEST( BlurBitmap, TooSmallBitmapsAreLeftAlone )
{
  TestBitmap tiny( 2, 2, 0x00102030 );
  tiny.At( 0, 0 ) = 0x00FFFFFF;
  tiny.Blur( 4 );
  EXPECT_EQ( tiny.At( 0, 0 ), 0x00FFFFFFu );
  EXPECT_EQ( tiny.At( 1, 1 ), 0x00102030u );

  TestBitmap line( 10, 1, 0x00102030 );
  line.Blur( 4 );
  EXPECT_EQ( line.At( 5, 0 ), 0x00102030u );

  // zero sized client area, e.g. minimized window
  HDC screen = GetDC( NULL );
  WindowsUtils::BlurBitmap( screen, tiny.bitmap, 0, 0, 4 );
  ReleaseDC( NULL, screen );
  GdiFlush();
  EXPECT_EQ( tiny.At( 0, 0 ), 0x00FFFFFFu );
}

TEST( BlurBitmap, ZeroPassesKeepsImage )
{
  TestBitmap image( 5, 5, 0 );
  image.At( 2, 2 ) = 0x00FFFFFF;
  image.Blur( 0 );
  EXPECT_EQ( image.At( 2, 2 ), 0x00FFFFFFu );
  EXPECT_EQ( image.At( 1, 2 ), 0u );
}
