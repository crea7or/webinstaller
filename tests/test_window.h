// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <windows.h>
#include <windowsx.h>

// hidden top level window used as a parent for controls under test
// it is never shown and its messages are never dispatched by tests,
// so no painting happens - posted notifications are read with PeekMessage
class TestParentWindow
{
public:
  TestParentWindow()
  {
    static const wchar_t className[] = L"WebInstallerTestParent";
    static bool registered = false;
    if ( !registered )
    {
      WNDCLASSEX windowClass = { sizeof( WNDCLASSEX ) };
      windowClass.lpfnWndProc = DefWindowProc;
      windowClass.hInstance = GetModuleHandle( NULL );
      windowClass.lpszClassName = className;
      RegisterClassEx( &windowClass );
      registered = true;
    }
    handle = CreateWindowEx( 0, className, L"", WS_POPUP, 0, 0, 400, 200, NULL, NULL, GetModuleHandle( NULL ), NULL );
  }

  ~TestParentWindow()
  {
    DestroyWindow( handle );
    // drop anything left in the queue for this window
    MSG msg;
    while ( PeekMessage( &msg, NULL, WM_COMMAND, WM_COMMAND, PM_REMOVE ))
    {}
  }

  // creates STATIC child window
  HWND CreateStatic( UINT id, const wchar_t* text, DWORD style = SS_LEFT | SS_NOTIFY, int width = 300, int height = 20 )
  {
    return CreateWindowEx( 0, L"STATIC", text, WS_CHILD | WS_VISIBLE | style, 0, 0, width, height, handle, ( HMENU )( UINT_PTR ) id, GetModuleHandle( NULL ), NULL );
  }

  // takes the next posted WM_COMMAND, returns false if there is none
  bool TakeCommand( WPARAM& wParam, LPARAM& lParam )
  {
    MSG msg;
    if ( PeekMessage( &msg, handle, WM_COMMAND, WM_COMMAND, PM_REMOVE ))
    {
      wParam = msg.wParam;
      lParam = msg.lParam;
      return true;
    }
    return false;
  }

  bool HasCommand()
  {
    MSG msg;
    return PeekMessage( &msg, handle, WM_COMMAND, WM_COMMAND, PM_NOREMOVE ) != FALSE;
  }

  HWND handle;
};

// simulated mouse input sent directly to the control window procedure
inline void SendMouse( HWND window, UINT message, int x, int y = 5 )
{
  SendMessage( window, message, message == WM_MOUSEMOVE ? 0 : MK_LBUTTON, MAKELPARAM( x, y ));
}

inline void Click( HWND window, int x, int y = 5 )
{
  SendMouse( window, WM_LBUTTONDOWN, x, y );
  SendMouse( window, WM_LBUTTONUP, x, y );
}
