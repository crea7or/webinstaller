// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <windows.h>
#include <windowsx.h>

#include <string>

#include "windows/windows_utils.h"
#include "installer/system_utils.h"

enum INSTALLER_MSG : UINT
{
  SET_PROGRESS_TEXT = WM_USER + 42,
  SET_PROGRESS_BAR,

  SETUP_COMPLETE,
  SETUP_CANCELLED,
  SETUP_ERROR,

  EULA_CLICK,
  POLICY_CLICK,
};

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

class AbstractDialog
{
public:
  AbstractDialog( HINSTANCE _instanceHandle, UINT _templateId, HWND _parentHandle = NULL ) :
    instanceHandle( _instanceHandle ), templateId( _templateId ), parentHandle( _parentHandle ), currentDpi( 96 )
  {}

  virtual ~AbstractDialog()
  {
    if ( dialogHandle != NULL )
    {
      DestroyWindow( dialogHandle );
    }
  }

  bool Create()
  {
    dialogHandle = CreateDialogParam( instanceHandle, MAKEINTRESOURCE( templateId ), parentHandle, DialogFunction, ( LPARAM )this );
    return dialogHandle != NULL;
  }

  INT_PTR CreateModal()
  {
    return DialogBoxParam( instanceHandle, MAKEINTRESOURCE( templateId ), parentHandle, DialogFunction, ( LPARAM )this );
  }

  virtual LRESULT OnCtrlColor( UINT, WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnDrawItem( WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnEraseBackground( WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnMouse( UINT, WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnUserMessage( UINT, WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnCommand( WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnTimer( WPARAM, LPARAM ) { return 0; };
  virtual LRESULT OnClose() { return 0; };
  virtual void LoadFonts() {};

  virtual LRESULT OnInitDialog( HWND window )
  {
    // set our HWND
    dialogHandle = window;
    SetWindowLongPtr( dialogHandle, GWLP_USERDATA, ( LONG_PTR )this );

    // initial window rectangle
    WindowsUtils::RectHolder initialWindowRect;
    GetWindowRect( dialogHandle, &initialWindowRect );

    HINSTANCE moduleHandle = GetModuleHandle( NULL );
    if ( moduleHandle != NULL )
    {
      HRSRC resourceHandle = FindResource( moduleHandle, MAKEINTRESOURCE( templateId ), RT_DIALOG );
      if ( resourceHandle != NULL )
      {
        HGLOBAL templateHandle = LoadResource( moduleHandle, resourceHandle );
        if ( templateHandle != NULL )
        {
          DLGTEMPLATEEX* templatePtr = ( DLGTEMPLATEEX* ) LockResource( templateHandle );
          if ( templatePtr != NULL )
          {
            // dialog scale
            // get original size and position in DLU
            WindowsUtils::RectHolder windowDluRect;
            windowDluRect.left = templatePtr->x;
            windowDluRect.right = templatePtr->x + templatePtr->cx;
            windowDluRect.top = templatePtr->y;
            windowDluRect.bottom = templatePtr->y + templatePtr->cy;

            // translate to default DPI 96 values
            windowDluRect.ScaleX( 3, 2 );
            windowDluRect.ScaleY( 13, 8 );

            // move to initial position
            windowDluRect.Move( initialWindowRect.left - (( windowDluRect.Width() - initialWindowRect.Width()) / 2 ), initialWindowRect.top - (( windowDluRect.Height() - initialWindowRect.Height()) / 2 ));
            // DLU related scale and move
            SetWindowPos( dialogHandle, NULL, windowDluRect.left, windowDluRect.top, windowDluRect.Width(), windowDluRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
            // assign to use in DPI scaling
            initialWindowRect = windowDluRect;

            // dialog items scale
            DLGITEMTEMPLATEEX* itemTemplatePtr = WindowsUtils::GetFirstItemTemplateEx( templatePtr );
            HWND child;
            WindowsUtils::RectHolder childDluRect;
            for ( WORD item = 0; item < templatePtr->cDlgItems; ++item )
            {
              child = GetDlgItem( dialogHandle, itemTemplatePtr->id );
              if ( child != NULL )
              {
                // original size and position in DLU
                childDluRect.left = itemTemplatePtr->x;
                childDluRect.right = itemTemplatePtr->x + itemTemplatePtr->cx;
                childDluRect.top = itemTemplatePtr->y;
                childDluRect.bottom = itemTemplatePtr->y + itemTemplatePtr->cy;

                // translate to default DPI 96 values
                childDluRect.ScaleX( 3, 2 );
                childDluRect.ScaleY( 13, 8 );

                // DLU related scale and move
                SetWindowPos( child, NULL, childDluRect.left, childDluRect.top, childDluRect.Width(), childDluRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
              }
              itemTemplatePtr = WindowsUtils::GetNextItemTemplateEx( itemTemplatePtr );
            }
          }
          UnlockResource( templateHandle );
          ::FreeResource( templateHandle );
        }
      }
    }

    // update DPI to the current window DPI
    OnDpiChange( WindowsUtils::GetWindowDpi( dialogHandle ), NULL );

    return 0;
  }

  virtual LRESULT OnDpiChange( WPARAM wParam, LPARAM lParam )
  {
    UINT newDpi = (( UINT ) wParam ) & 0xFFFF;

    // actually reload if called after init
    LoadFonts();

    // scale window
    if ( lParam != NULL )
    {
      // use system supplied RECT
      WindowsUtils::RectHolder* suggestedRectPtr = ( WindowsUtils::RectHolder* ) lParam;
      SetWindowPos( dialogHandle, NULL, suggestedRectPtr->left, suggestedRectPtr->top, suggestedRectPtr->Width(), suggestedRectPtr->Height(), SWP_NOZORDER | SWP_NOACTIVATE );
    }
    else
    {
      // calculate it for initial update
      WindowsUtils::RectHolder currentRect, newWindowRect;
      GetWindowRect( dialogHandle, &newWindowRect );
      currentRect = newWindowRect;
      newWindowRect.Scale( newDpi, currentDpi );
      newWindowRect.Move( currentRect.left - (( newWindowRect.Width() - currentRect.Width()) / 2 ), currentRect.top - (( newWindowRect.Height() - currentRect.Height()) / 2 ));
      SetWindowPos( dialogHandle, NULL, newWindowRect.left, newWindowRect.top, newWindowRect.Width(), newWindowRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
    }

    // scale child windows
    std::vector< HWND > childs;
    EnumChildWindows( dialogHandle, ChildWindowsEnumCallback, ( LPARAM ) &childs );
    for ( HWND child : childs )
    {
      WindowsUtils::RectHolder childRect;
      GetWindowRect( child, &childRect );
      MapWindowPoints( HWND_DESKTOP, dialogHandle, ( LPPOINT ) &childRect, 2 );
      childRect.Scale( newDpi, currentDpi );
      SetWindowPos( child, NULL, childRect.left, childRect.top, childRect.Width(), childRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
    }

    RedrawWindow( dialogHandle, NULL, NULL, RDW_ERASE | RDW_INTERNALPAINT | RDW_INVALIDATE );
    currentDpi = newDpi;
    return ( LRESULT ) TRUE;
  }

  static BOOL CALLBACK ChildWindowsEnumCallback( HWND handle, LPARAM lParam )
  {
    if ( lParam != NULL )
    {
      std::vector< HWND >* childs = reinterpret_cast< std::vector< HWND >*>( lParam );
      childs->push_back( handle );
    }
    return TRUE;
  }

  static INT_PTR CALLBACK DialogFunction( HWND dialogHandle, UINT message, WPARAM wParam, LPARAM lParam )
  {
    AbstractDialog* dialog = reinterpret_cast<AbstractDialog*>( GetWindowLongPtr( dialogHandle, GWLP_USERDATA ));
    LRESULT result = 0;

    if ( dialog != NULL )
    {
      if ( message > WM_USER )
      {
        result = dialog->OnUserMessage( message, wParam, lParam );
      }
      else
      {
        switch ( message )
        {
          case WM_LBUTTONDOWN:
          case WM_LBUTTONUP:
          case WM_MOUSEMOVE:
            result = dialog->OnMouse( message, wParam, lParam );
            break;

          case WM_CTLCOLOR:
          case WM_CTLCOLORBTN:
          case WM_CTLCOLORSTATIC:
            result = dialog->OnCtrlColor( message, wParam, lParam );
            break;

          case WM_ERASEBKGND:
            result = dialog->OnEraseBackground( wParam, lParam );
            break;

          case WM_DRAWITEM:
            result = dialog->OnDrawItem( wParam, lParam );
            break;

          case WM_COMMAND:
            result = dialog->OnCommand( wParam, lParam );
            break;

          case WM_DPICHANGED:
            result = dialog->OnDpiChange( wParam, lParam );
            break;

          case WM_TIMER:
            if ( wParam == ( WPARAM) dialogHandle )
            {
              result = dialog->OnTimer( wParam, lParam );
            }
            break;

          case WM_CLOSE:
            result = dialog->OnClose();
            break;
        }
      }
    }
    else if ( message == WM_INITDIALOG && lParam != NULL )
    {
      // workaround for WM_INITDIALOG processing before actual creation of dialog
      dialog = reinterpret_cast<AbstractDialog*>( lParam );
      result = dialog->OnInitDialog( dialogHandle );
    }
    else if ( message == WM_ERASEBKGND )
    {
      // workaround for WM_ERASEBKGND processing before actual creation of dialog
    }
    return result;
  };

  bool SetFont( UINT id, HFONT font )
  {
    SendMessage( GetDlgItem( dialogHandle, id ), WM_SETFONT, (LPARAM) font, true );
    return true;
  }

  HWND GetDialogHandle()
  {
    return dialogHandle;
  }

  void DestroyDialog( bool quit = false )
  {
    SetWindowLongPtr( dialogHandle, GWLP_USERDATA, ( LONG_PTR )NULL );
    DestroyWindow( dialogHandle );
    dialogHandle = NULL;
    if ( quit )
    {
      PostQuitMessage( 0 );
    }
  }

  bool ShowDialog( int showMode = SW_NORMAL )
  {
    RedrawDialog( true );
    return ::ShowWindow( dialogHandle, showMode ) == TRUE;
  }

  bool UpdateDialog()
  {
    return ::UpdateWindow( dialogHandle ) == TRUE;
  }

  void RedrawDialog( bool erase = false )
  {
    RedrawWindow( dialogHandle, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE | ( erase ? RDW_ERASE : 0 ));
  }

protected:

  // create params
  HWND dialogHandle;
  HINSTANCE instanceHandle;
  UINT templateId;
  HWND parentHandle;
  // create params

  // HiDPI functionality
  UINT currentDpi;
};

