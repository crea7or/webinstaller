// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "installer/system_utils.h"

namespace SystemUtils
{
  typedef enum _MONITOR_DPI_TYPE
  {
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
  } MONITOR_DPI_TYPE;

  UINT GetDpiForMonitorShcore( HWND window )
  {
    UINT result = 0;
    RECT windowRect = { 0, 0, 0, 0 };
    if ( window != NULL )
    {
      GetWindowRect( window, &windowRect );
    }
    POINT dotPoint;
    dotPoint.x = windowRect.left;
    dotPoint.y = windowRect.top;
    HMONITOR currentMonitor = MonitorFromPoint( dotPoint, MONITOR_DEFAULTTONEAREST );
    if ( currentMonitor != NULL )
    {
      typedef HRESULT( WINAPI *LPFN_GETDPIFORMONITOR )( HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT* );
      HMODULE shcodeDll = LoadLibrary( L"shcore.dll" );
      if ( shcodeDll != NULL )
      {
        UINT dpiX = 0, dpiY = 0;
        LPFN_GETDPIFORMONITOR fnGetDpiForMonitor = ( LPFN_GETDPIFORMONITOR ) GetProcAddress( shcodeDll, "GetDpiForMonitor" );
        if ( fnGetDpiForMonitor != NULL )
        {
          if ( fnGetDpiForMonitor( currentMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY ) == S_OK )
          {
            result = dpiX;
          }
        }
        FreeLibrary( shcodeDll );
      }
    }
    return result;
  }

  UINT GetDpiForWindowUser32( HWND window )
  {
    UINT result = 0;
    typedef UINT( WINAPI *LPFN_GETDPIFORWINDOW )( HWND );
    LPFN_GETDPIFORWINDOW fnGetDpiForWindow = (LPFN_GETDPIFORWINDOW) GetProcAddress( GetModuleHandle( L"user32" ), "GetDpiForWindow" );
    if ( fnGetDpiForWindow != NULL )
    {
      result = fnGetDpiForWindow( window );
    }
    return result;
  }
}