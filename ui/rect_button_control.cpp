// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/rect_button_control.h"
#include "windows/windows_utils.h"

RectButtonControl::RectButtonControl()
{
  // invalid value by default
  textShadowColor = MAXDWORD;
  // text only button by default
  buttonNormalColor = MAXDWORD;
}

RectButtonControl::~RectButtonControl()
{
}

LRESULT RectButtonControl::OnLeftButtonDown( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
  SetState( STATE::CLICK );
  return (LRESULT) 1;
}

LRESULT RectButtonControl::OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam )
{
  return OnLeftButtonDown( wParam, lParam );
}

LRESULT RectButtonControl::OnLeftButtonUp( WPARAM, LPARAM lParam )
{
  if ( currentState == STATE::CLICK )
  {
    SetState( STATE::NORMAL );
    HWND parent = GetParent( windowHandle );
    if ( parent != NULL )
    {
      PostMessage( parent, WM_COMMAND, WORD( GetDlgCtrlID( windowHandle )), lParam );
    }
  }
  else
  {
    SetState( STATE::NORMAL );
  }
  return (LRESULT) 1;
}

LRESULT RectButtonControl::OnPaint()
{
  PAINTSTRUCTEX psEx;
  Prepaint( psEx );

  // paint button background
  if ( buttonNormalColor != MAXDWORD )
  {
    int scale = 2;
    WindowsUtils::RectHolder aliasedRect = psEx.windowRectangle;
    aliasedRect.right *= scale;
    aliasedRect.bottom *= scale;
    HDC aliasingContext = CreateCompatibleDC( psEx.memoryContext );
    HBITMAP aliasingBitmap = CreateCompatibleBitmap( psEx.memoryContext, aliasedRect.Width(), aliasedRect.Height());
    HBITMAP previousBitmap = ( HBITMAP ) SelectObject( aliasingContext, aliasingBitmap );
    StretchBlt( aliasingContext, 0, 0, aliasedRect.Width(), aliasedRect.Height(), psEx.memoryContext, 0, 0, psEx.windowRectangle.Width(), psEx.windowRectangle.Height(), SRCCOPY );

    COLORREF rectColor = buttonNormalColor;
    if ( currentState == STATE::HOVER )
    {
      rectColor = buttonHoverColor;
    }
    else if ( currentState == STATE::CLICK )
    {
      rectColor = buttonClickColor;
    }
    int roundingDpi = WindowsUtils::MultiplyThenDivide( 10, WindowsUtils::GetWindowDpi( windowHandle ), 96 );
    int rectRounding = ( int )( roundingDpi * scale );

    HBRUSH rectColorBrush = CreateSolidBrush( rectColor );
    HBRUSH previousBrush = ( HBRUSH )SelectObject( aliasingContext, rectColorBrush );
    HPEN rectPen = CreatePen( PS_NULL, 0, rectColor );
    HPEN previousPen = ( HPEN )SelectObject( aliasingContext, rectPen );
    WindowsUtils::RectHolder buttonRect = aliasedRect;

    // make rect smaller than window
    buttonRect.right -= 6;
    buttonRect.bottom -= 6;

    // increase position to draw clicked image
    if ( currentState == STATE::CLICK )
    {
      buttonRect.left += 2;
      buttonRect.top += 2;
      buttonRect.right += 2;
      buttonRect.bottom += 2;
    }
    // draw button itself
    RoundRect( aliasingContext, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom, rectRounding, rectRounding );

    SelectObject( aliasingContext, previousBrush );
    SelectObject( aliasingContext, previousPen );
    DeleteObject( rectColorBrush );
    DeleteObject( rectPen );

    int previousMode = SetStretchBltMode( psEx.memoryContext, HALFTONE );
    StretchBlt( psEx.memoryContext, 0, 0, psEx.windowRectangle.Width(), psEx.windowRectangle.Height(), aliasingContext, 0, 0, aliasedRect.Width(), aliasedRect.Height(), SRCCOPY );
    SetStretchBltMode( psEx.memoryContext, previousMode );

    SelectObject( aliasingContext, previousBitmap );
    DeleteObject( aliasingBitmap );
    DeleteDC( aliasingContext );
  }
  // paint button background

  DWORD outerStyle = GetControlStyles( DT_EDITCONTROL | DT_VCENTER | DT_SINGLELINE );
  COLORREF textColor = GetControlTextColor();

  psEx.windowRectangle.top += 2;
  psEx.windowRectangle.bottom += 2;
  if ( currentState == STATE::CLICK )
  {
    // offset for clicked text
    ++psEx.windowRectangle.top;
    ++psEx.windowRectangle.bottom;
    ++psEx.windowRectangle.left;
    ++psEx.windowRectangle.right;
  }

  psEx.previousFont = ( HFONT ) SelectObject( psEx.memoryContext, textFont );
  psEx.windowRectangle.top -= 2;
  psEx.windowRectangle.bottom -= 2;
  if ( textShadowColor != MAXDWORD )
  {
    // draw text shadow if needed
    SetTextColor( psEx.memoryContext, textShadowColor );
    DrawText( psEx.memoryContext, windowText.c_str(), -1, &psEx.windowRectangle, outerStyle );
    --psEx.windowRectangle.top;
    --psEx.windowRectangle.bottom;
    --psEx.windowRectangle.left;
    --psEx.windowRectangle.right;
  }
  // draw test itself
  SetTextColor( psEx.memoryContext, textColor );
  DrawText( psEx.memoryContext, windowText.c_str(), -1, &psEx.windowRectangle, outerStyle );

  Postpaint( psEx );

  return 0;
}
