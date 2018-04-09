// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <windows.h>
#include <string>

class BaseControl
{
public:
  BaseControl();
  virtual ~BaseControl();

  bool Attach( const HWND ownerWindow, const int windowId );
  bool Attach( const HWND window );
  bool Detach();
  bool IsAttached() const
  {
    return prevoiusWndProc != NULL ? true : false;
  }

  static LRESULT CALLBACK ControlProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam );
  LRESULT PreviousControlProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam );

  virtual LRESULT OnEraseBackground( HDC deviceContext );
  virtual LRESULT OnPaint() { return 0; };
  virtual LRESULT OnMouseHover( WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnMouseMove( WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnMouseLeave() { return 0; };
  virtual LRESULT OnSetCursor() { return 0; };
  virtual LRESULT OnLeftButtonDown( WPARAM wParam, LPARAM lParam );
  virtual LRESULT OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam );
  virtual LRESULT OnLeftButtonUp( WPARAM wParam, LPARAM lParam );

  void Show( bool action );

  void SetForegroundColor( const COLORREF color )
  {
    foregroundColor = color;
  }
  COLORREF GetForegroundColor() const
  {
    return foregroundColor;
  }

  void SetBackgroundColor( const COLORREF color )
  {
    backgroundColor = color;
    windowBackgroundBrush = CreateSolidBrush( color );
  }
  COLORREF GetBackgroundColor() const
  {
    return backgroundColor;
  }

  HWND GetWindow()
  {
    return windowHandle;
  }

  virtual void SetFont( LOGFONT* fontStruct );
  virtual std::wstring GetText();
  virtual void SetText( const std::wstring& text );

  void RedrawControl( bool erase = false )
  {
    RedrawWindow( windowHandle, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE | ( erase ? RDW_ERASE : 0 ));
  }

protected:

  void SetFont( HFONT fontHandle );
  void DestroyFont();

  COLORREF foregroundColor;
  COLORREF disabledForegroundColor;
  COLORREF backgroundColor;

  HBRUSH windowBackgroundBrush;

  // current window handle
  HWND windowHandle;

  // previous windows procedure
  WNDPROC prevoiusWndProc;

  // default font
  HFONT textFont;

  // windows text that will be captured on Attach
  std::wstring windowText;
};
