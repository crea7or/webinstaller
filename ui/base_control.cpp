// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/base_control.h"
#include "windows/abstract_dialog.h"
#include "installer/installer_constants.h"
#include "windows/windows_utils.h"

BaseControl::BaseControl()
{
  foregroundColor = GetSysColor( COLOR_WINDOWTEXT );
  backgroundColor = GetSysColor( COLOR_WINDOW );
  windowBackgroundBrush = CreateSolidBrush( backgroundColor );
  prevoiusWndProc = NULL;
  textFont = NULL;
  windowHandle = NULL;

  LOGFONT textLogFont;
  WindowsUtils::FillLogFont( textLogFont, 10 );
  SetFont( &textLogFont );
}

BaseControl::~BaseControl()
{
  Detach();
  DeleteObject( windowBackgroundBrush );
  DestroyFont();
}

void BaseControl::SetFont( LOGFONT* fontStruct )
{
  DestroyFont();
  textFont = CreateFontIndirect( fontStruct );
}

void BaseControl::SetFont( HFONT fontHandle)
{
  DestroyFont();
  textFont = fontHandle;
}

void BaseControl::DestroyFont()
{
  if ( textFont != NULL )
  {
    DeleteObject( textFont );
    textFont = NULL;
  }
}

LRESULT CALLBACK BaseControl::ControlProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
  BaseControl* controlPtr = ( BaseControl* )GetWindowLongPtr( window, GWLP_USERDATA );
  LRESULT result = 0;
  if ( controlPtr != NULL )
  {
    switch ( message )
    {
      case WM_DESTROY:
      case WM_CLOSE:
        controlPtr->Detach();
        break;
      case WM_ERASEBKGND:
        result = controlPtr->OnEraseBackground(( HDC )wParam );
        break;
      case WM_PAINT:
        result = controlPtr->OnPaint();
        break;
      case WM_SETFONT:
        if ( wParam != NULL )
        {
          controlPtr->SetFont(( HFONT )wParam );
          if ( LOWORD( lParam ) > 0 )
          {
            result = controlPtr->OnPaint();
          }
        }
        break;
      case WM_MOUSEHOVER:
        result = controlPtr->OnMouseHover( wParam, lParam );
        break;
      case WM_MOUSEMOVE:
      case WM_NCMOUSEMOVE:
        result = controlPtr->OnMouseMove( wParam, lParam );
        break;
      case WM_MOUSELEAVE:
      case WM_NCMOUSELEAVE:
        result = controlPtr->OnMouseLeave();
        break;
      case WM_LBUTTONDOWN:
        result = controlPtr->OnLeftButtonDown( wParam, lParam );
        break;
      case WM_LBUTTONDBLCLK:
        result = controlPtr->OnLeftButtonDoubleDown( wParam, lParam );
        break;
      case WM_LBUTTONUP:
        result = controlPtr->OnLeftButtonUp( wParam, lParam );
        break;
      case WM_SETCURSOR:
        result = controlPtr->OnSetCursor();
        break;
      default:
        result = controlPtr->PreviousControlProc( window, message, wParam, lParam );
    }
  }
  else
  {
    result = DefWindowProc( window, message, wParam, lParam );
  }
  return result;
}

LRESULT BaseControl::OnLeftButtonDown( WPARAM wParam, LPARAM lParam )
{
  return PreviousControlProc( windowHandle, WM_LBUTTONDOWN, wParam, lParam );
}

LRESULT BaseControl::OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam )
{
  return PreviousControlProc( windowHandle, WM_LBUTTONDBLCLK, wParam, lParam );
}

LRESULT BaseControl::OnLeftButtonUp( WPARAM wParam, LPARAM lParam )
{
  return PreviousControlProc( windowHandle, WM_LBUTTONUP, wParam, lParam );
}

LRESULT BaseControl::PreviousControlProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
  if ( prevoiusWndProc != NULL )
  {
    return CallWindowProc( prevoiusWndProc, window, message, wParam, lParam );
  }
  else
  {
    return DefWindowProc( window, message, wParam, lParam );
  }
}

std::wstring BaseControl::GetText()
{
  std::wstring text;
  int textSize = GetWindowTextLength( windowHandle );
  if ( textSize > 0 )
  {
    ++textSize; // because \0 is not included
    text.assign( textSize, ( wchar_t ) 0 );
    textSize = GetWindowText( windowHandle, const_cast< wchar_t* >( text.c_str()), textSize );
  }
  if ( textSize == 0 )
  {
    text.clear();
  }
  return text;
}

void BaseControl::SetText( const std::wstring& text )
{
  SetWindowText( windowHandle, text.c_str());
}

bool BaseControl::Attach( const HWND ownerWindow, const int windowId )
{
  return Attach( GetDlgItem( ownerWindow, windowId ));
}

bool BaseControl::Attach( const HWND window )
{
  if ( window == NULL || prevoiusWndProc != NULL )
  {
    return false;
  }
  windowHandle = window;
  prevoiusWndProc = ( WNDPROC )SetWindowLongPtr( window, GWLP_WNDPROC, ( LONG )BaseControl::ControlProc );
  SetWindowLongPtr( window, GWLP_USERDATA, ( LONG_PTR )this );
  windowText = GetText();
  return TRUE;
}

bool BaseControl::Detach()
{
  if ( windowHandle == NULL || prevoiusWndProc == NULL )
  {
    return false;
  }
  SetWindowLongPtr( windowHandle, GWLP_WNDPROC, ( LONG )prevoiusWndProc );
  SetWindowLongPtr( windowHandle, GWLP_USERDATA, ( LONG )NULL );
  prevoiusWndProc = NULL;
  windowHandle = NULL;
  return TRUE;
}

LRESULT BaseControl::OnEraseBackground( HDC deviceContext )
{
  if ( deviceContext != NULL )
  {
    RECT mainWindow;
    GetClientRect( windowHandle, &mainWindow );
    FillRect( deviceContext, &mainWindow, windowBackgroundBrush );
  }
  return ( LRESULT )TRUE;
}

void BaseControl::Show( bool action )
{
  if ( action )
  {
    ShowWindow( windowHandle, SW_SHOWNORMAL );
  }
  else
  {
    ShowWindow( windowHandle, SW_HIDE );
  }
}
