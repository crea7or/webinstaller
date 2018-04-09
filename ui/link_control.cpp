// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/link_control.h"

LinkControl::LinkControl()
{
  hoverColor = RGB( 0x30, 0x88, 0xe0 );
  previousCursor = NULL;
  linkCursor = LoadCursor( NULL, IDC_HAND );
}

LinkControl::~LinkControl()
{
  DestroyCursor( linkCursor );
  DestroyBackground();
}

LRESULT LinkControl::OnMouseMove( WPARAM, LPARAM )
{
  if ( previousCursor == NULL && currentState != STATE::CLICK )
  {
    mouseEvent.cbSize = sizeof( TRACKMOUSEEVENT );
    mouseEvent.dwFlags = TME_LEAVE;
    mouseEvent.hwndTrack = windowHandle;
    TrackMouseEvent( &mouseEvent );
    SetState( STATE::HOVER );
  }
  return 0;
}

LRESULT LinkControl::OnMouseLeave()
{
  SetState( STATE::NORMAL );
  return 0;
}

void LinkControl::SetState( STATE newState )
{
  if ( newState != currentState )
  {
    switch ( newState )
    {
      case STATE::HOVER:
      {
        if ( previousCursor == NULL )
        {
          previousCursor = SetCursor( linkCursor );
        }
        break;
      }
      case STATE::NORMAL:
      {
        if ( previousCursor != NULL )
        {
          SetCursor( previousCursor );
          previousCursor = NULL;
          ShowCursor( TRUE );
        }
        break;
      }
      case STATE::CLICK:
      {
        break;
      }
    };

    RedrawControl();

    currentState = newState;
  }
}

LRESULT LinkControl::OnLeftButtonDown( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
  SetState( STATE::CLICK );
  return (LRESULT) 1;
}

LRESULT LinkControl::OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam )
{
  return OnLeftButtonDown( wParam, lParam );
}

LRESULT LinkControl::OnLeftButtonUp( WPARAM, LPARAM lParam )
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
