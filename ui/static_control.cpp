// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/static_control.h"

StaticControl::StaticControl()
{
  currentState = STATE::NORMAL;
  hoverColor = GetSysColor( COLOR_HOTLIGHT );
  clickColor = GetSysColor( COLOR_3DFACE );

  // in memory background copy
  clearBackgroundBitmap = NULL;
  backgroundContext = NULL;
  oldBackgroundBitmap = NULL;
  // in memory background copy
}

StaticControl::~StaticControl()
{
  DestroyBackground();
}

void StaticControl::DestroyBackground()
{
  if ( backgroundContext != NULL )
  {
    // in memory background copy
    SelectObject( backgroundContext, oldBackgroundBitmap );
    DeleteObject( clearBackgroundBitmap );
    DeleteDC( backgroundContext );
    // in memory background copy
  }
  clearBackgroundBitmap = NULL;
  backgroundContext = NULL;
  oldBackgroundBitmap = NULL;
}

std::wstring StaticControl::GetText()
{
  if ( windowText.empty())
  {
    windowText = BaseControl::GetText();
  }
  return windowText;
}

void StaticControl::SetText( const std::wstring& text )
{
  windowText.assign( text );
  RedrawControl();
}

DWORD StaticControl::GetControlStyles( const DWORD initialStyle ) const
{
  DWORD style = initialStyle;
  DWORD controlStyle = GetWindowLong( windowHandle, GWL_STYLE );
  if (( controlStyle & SS_CENTER ) == SS_CENTER )
  {
    style |= DT_CENTER;
  }
  else
  {
    style |= ( controlStyle & SS_RIGHT ) == SS_RIGHT ? DT_RIGHT : DT_LEFT;
  }
  return style;
}

COLORREF StaticControl::GetControlTextColor() const
{
  COLORREF textColor = foregroundColor;
  if ( IsWindowEnabled( windowHandle ) == FALSE )
  {
    textColor = disabledForegroundColor;
  }
  else if ( currentState == STATE::HOVER )
  {
    textColor = hoverColor;
  }
  else if ( currentState == STATE::CLICK )
  {
    textColor = clickColor;
  }
  return textColor;
}

void StaticControl::Prepaint( PAINTSTRUCTEX& psEx )
{
  BeginPaint( windowHandle, &psEx );
  GetClientRect( windowHandle, &psEx.windowRectangle );

  // memory bitmap
  psEx.memoryContext = CreateCompatibleDC( psEx.hdc );
  psEx.memoryBitmap = CreateCompatibleBitmap( psEx.hdc, psEx.windowRectangle.right - psEx.windowRectangle.left, psEx.windowRectangle.bottom - psEx.windowRectangle.top );
  psEx.previousMemoryBitmap = ( HBITMAP )SelectObject( psEx.memoryContext, psEx.memoryBitmap );
  // memory bitmap

  // memory background bitmap
  if ( backgroundContext == NULL )
  {
    backgroundContext = CreateCompatibleDC( psEx.hdc );
    clearBackgroundBitmap = CreateCompatibleBitmap( psEx.hdc, psEx.windowRectangle.right - psEx.windowRectangle.left, psEx.windowRectangle.bottom - psEx.windowRectangle.top );
    oldBackgroundBitmap = ( HBITMAP )SelectObject( backgroundContext, clearBackgroundBitmap );
    BitBlt( backgroundContext, 0, 0, psEx.windowRectangle.right - psEx.windowRectangle.left, psEx.windowRectangle.bottom - psEx.windowRectangle.top, psEx.hdc, 0, 0, SRCCOPY );
  }
  // memory background bitmap
  // paint over saved background
  BitBlt( psEx.memoryContext, 0, 0, psEx.windowRectangle.right - psEx.windowRectangle.left, psEx.windowRectangle.bottom - psEx.windowRectangle.top, backgroundContext, 0, 0, SRCCOPY );
  psEx.previousBkMode = SetBkMode( psEx.memoryContext, TRANSPARENT );
}

LRESULT StaticControl::OnPaint()
{
  PAINTSTRUCTEX psEx;
  Prepaint( psEx );

  SetTextColor( psEx.memoryContext, GetControlTextColor());
  psEx.previousFont = ( HFONT )SelectObject( psEx.memoryContext, textFont );
  DrawText( psEx.memoryContext, windowText.c_str(), -1, &psEx.windowRectangle, GetControlStyles( DT_EDITCONTROL | DT_WORDBREAK | DT_TOP ));

  Postpaint( psEx );
  return 0;
}

void StaticControl::Postpaint( PAINTSTRUCTEX& paintStruct )
{
  // memory bitmap copy
  BitBlt( paintStruct.hdc, 0, 0, paintStruct.windowRectangle.right - paintStruct.windowRectangle.left, paintStruct.windowRectangle.bottom - paintStruct.windowRectangle.top, paintStruct.memoryContext, 0, 0, SRCCOPY );
  // do not reorder copying and cleanup
  SelectObject( paintStruct.memoryContext, paintStruct.previousMemoryBitmap );
  SetBkMode( paintStruct.memoryContext, paintStruct.previousBkMode );
  DeleteObject( paintStruct.memoryBitmap );
  DeleteDC( paintStruct.memoryContext );
  // memory bitmap copy

  EndPaint( windowHandle, &paintStruct );
}

LRESULT StaticControl::OnEraseBackground( HDC dc )
{
  DestroyBackground();
  if ( windowText.size() == 0 )
  {
    return BaseControl::OnEraseBackground( dc );
  }
  else
  {
    return (LRESULT) TRUE;
  }
}