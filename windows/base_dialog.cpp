// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"

#include "windows/base_dialog.h"
#include "windows/windows_utils.h"
#include "installer/resource.h"

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

BaseDialog::BaseDialog( HINSTANCE _instanceHandle, UINT _templateId, HWND _parentHandle ) :
  AbstractDialog( _instanceHandle, _templateId, _parentHandle )
{
  // window move support
  windowIsMoving = false;
  positionChanged = false;
  pointerXPosition = 0;
  pointerYPosition = 0;
  // window move support

  // app name
  titleControl.SetForegroundColor( Installer::mainTextColor );

  // close cross
  closeLinkControl.SetForegroundColor( Installer::mainTextColor );

  windowBackgroundBrush = ( HBRUSH ) GetStockObject( WHITE_BRUSH );
  grayPen = CreatePen( PS_SOLID, 5, Installer::grayColor );
}

BaseDialog::~BaseDialog()
{
  DeleteObject( grayPen );
  DeleteObject( windowBackgroundBrush );
}

LRESULT BaseDialog::OnInitDialog( HWND initDialogHandle )
{
  AbstractDialog::OnInitDialog( initDialogHandle );

  // close cross - uses text only style
  closeLinkControl.SetButtonNormalColor( MAXDWORD );
  closeLinkControl.Attach( initDialogHandle, IDC_STATIC_CLOSE );

  // app logo
  logoControl.AddLayer( L"{", RGB( 0x99, 0xaa, 0xb5 ));
  logoControl.AddLayer( L"|", RGB( 0x27, 0x2b, 0x2b ));
  logoControl.AddLayer( L"}", RGB( 0xf4, 0xab, 0xba ));
  logoControl.AddLayer( L"~", RGB( 0x66, 0x75, 0x7f ));
  logoControl.Attach( initDialogHandle, IDC_STATIC_LOGO );

  // app name
  titleControl.Attach( initDialogHandle, IDC_STATIC_TEXT );

  return 0;
}

LRESULT BaseDialog::OnMouse( UINT message, WPARAM wParam, LPARAM lParam )
{
  RECT windowRectangle;
  POINT mousePointPosition;
  int windowXPosition = 0;
  int windowYPosition = 0;
  int oldX, oldY;

  switch ( message )
  {
    case WM_LBUTTONDOWN:
      windowXPosition = GET_X_LPARAM( lParam );
      windowYPosition = GET_Y_LPARAM( lParam );
      mousePointPosition.x = windowXPosition;
      mousePointPosition.y = windowYPosition;
      ClientToScreen( dialogHandle, &mousePointPosition );
      pointerXPosition = mousePointPosition.x;
      pointerYPosition = mousePointPosition.y;
      windowIsMoving = true;
      break;
    case WM_MOUSEMOVE:
      if (( wParam & MK_LBUTTON ) != 0 && windowIsMoving )
      {
        oldX = pointerXPosition;
        oldY = pointerYPosition;
        windowXPosition = GET_X_LPARAM( lParam );
        windowYPosition = GET_Y_LPARAM( lParam );
        mousePointPosition.x = windowXPosition;
        mousePointPosition.y = windowYPosition;
        ClientToScreen( dialogHandle, &mousePointPosition );
        pointerXPosition = mousePointPosition.x;
        pointerYPosition = mousePointPosition.y;
        GetWindowRect( dialogHandle, &windowRectangle );
        MoveWindow( dialogHandle, windowRectangle.left + pointerXPosition - oldX, windowRectangle.top + pointerYPosition - oldY, windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, TRUE );
        positionChanged = true;
      }
      else
      {
        windowIsMoving = false;
      }
      break;
    case WM_LBUTTONUP:
      oldX = pointerXPosition;
      oldY = pointerYPosition;
      windowXPosition = GET_X_LPARAM( lParam );
      windowYPosition = GET_Y_LPARAM( lParam );
      mousePointPosition.x = windowXPosition;
      mousePointPosition.y = windowYPosition;
      ClientToScreen( dialogHandle, &mousePointPosition );
      pointerXPosition = mousePointPosition.x;
      pointerYPosition = mousePointPosition.y;
      windowIsMoving = false;
      break;
  }
  return 0;
}

LRESULT BaseDialog::OnEraseBackground( WPARAM wParam, LPARAM )
{
  HDC deviceContext = ( HDC ) wParam;
  if ( deviceContext != NULL )
  {
    RECT mainWindow;
    GetClientRect( dialogHandle, &mainWindow );
#ifdef GRADIENT_BG
    // gradient background
    TRIVERTEX tv[ 2 ] =
    {
      { 1, 1, 0xee00, 0xab00, 0x3700 },
      { 100, 100, 0xff00, 0xe000, 0x8900 },
    };
    tv[ 0 ].x = mainWindow.left;
    tv[ 0 ].y = mainWindow.top;
    tv[ 1 ].x = mainWindow.right;
    tv[ 1 ].y = mainWindow.bottom;
    GRADIENT_RECT gr[ 1 ] = { { 0, 1 } };
    GradientFill( deviceContext, tv, 2, gr, 1, GRADIENT_FILL_RECT_V );
#else
    HPEN previousPen;
    HBRUSH previousBrush = ( HBRUSH ) SelectObject( deviceContext, windowBackgroundBrush );
    previousPen = ( HPEN ) SelectObject( deviceContext, grayPen );
    Rectangle( deviceContext, 0, 0, mainWindow.right - mainWindow.left, mainWindow.bottom - mainWindow.top );
    SelectObject( deviceContext, previousBrush );
    SelectObject( deviceContext, previousPen );
#endif
  }
  return ( LRESULT ) TRUE;
}

void BaseDialog::LoadFonts()
{
  LOGFONT fontStruct;
  WindowsUtils::FillLogFont( fontStruct, 23, dialogHandle );
  closeLinkControl.SetFont( &fontStruct );
  WindowsUtils::FillLogFont( fontStruct, 22, dialogHandle );
  titleControl.SetFont( &fontStruct );
  WindowsUtils::FillLogFont( fontStruct, 75, dialogHandle, NULL, true );
  logoControl.SetFont( &fontStruct );
}

bool BaseDialog::AskToCancelInstall()
{
  return MessageBox( dialogHandle, Installer::eMsgCancelMessage, Installer::eMsgWindowTitle, MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL ) == IDYES;
}
