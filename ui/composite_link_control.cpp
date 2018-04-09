// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/composite_link_control.h"

#include <windowsx.h>

CompositeLinkControl::CompositeLinkControl()
{
  textUnderlinedFont = NULL;
  drawRectangleOffset = 0;
  activePiece = -1;
}

CompositeLinkControl::~CompositeLinkControl()
{
  DestroyUnderlinedFont();
}

void CompositeLinkControl::DestroyUnderlinedFont()
{
  if ( textUnderlinedFont != NULL )
  {
    DeleteObject( textUnderlinedFont );
    textUnderlinedFont = NULL;
  }
}

bool CompositeLinkControl::AttachWithText( const HWND controlWindow )
{
  bool result = Attach( controlWindow );
  AddPiece( windowText );
  return result;
}

bool CompositeLinkControl::AttachWithText( const HWND ownerWindow, const int windowId )
{
  bool result = Attach( ownerWindow, windowId );
  AddPiece( windowText );
  return result;
}

void CompositeLinkControl::SetFont( LOGFONT* fontStruct )
{
  fontStruct->lfUnderline = TRUE;
  BaseControl::SetFont( fontStruct );

  DestroyUnderlinedFont();
  textUnderlinedFont = textFont;
  // set to null ot avoid font release by base class while new font will be set
  textFont = NULL;
  fontStruct->lfUnderline = FALSE;
  BaseControl::SetFont( fontStruct );
}

LRESULT CompositeLinkControl::OnPaint()
{
  if ( pieces.size() > 0 )
  {
    PAINTSTRUCTEX psEx;
    Prepaint( psEx );

    RECT drawRectangle = psEx.windowRectangle;

    // should be set before text size measurement
    psEx.previousFont = (HFONT) SelectObject( psEx.memoryContext, textFont );

    LONG totalWidth = CalculateCheckboxSize( psEx.memoryContext );
    LONG windowWidth = psEx.windowRectangle.right - psEx.windowRectangle.left;

    // calculate text sizes
    RECT textRect = { 0 };
    for ( std::vector< TextPiece >::iterator it = pieces.begin(); it != pieces.end(); ++it )
    {
      if ( DrawText( psEx.memoryContext, it->text.c_str(), -1, &textRect, DT_CALCRECT ) > 0 )
      {
        it->size.cx = textRect.right - textRect.left;
      }
      else
      {
        it->size.cx = 0;
        it->size.cy = 0;
      }
      totalWidth += it->size.cx;
    }

    // aligning options
    // multiline text is not supported yet
    DWORD outerStyle = DT_EDITCONTROL | DT_SINGLELINE | DT_VCENTER;
    DWORD controlStyle = GetWindowLong( windowHandle, GWL_STYLE );
    if ( totalWidth < windowWidth )
    {
      drawRectangleOffset = 0;
      if (( controlStyle & SS_CENTER ) == SS_CENTER )
      {
        // place at the center position
        drawRectangleOffset = ( windowWidth - totalWidth ) >> 1;
      }
      else if ( controlStyle & SS_RIGHT )
      {
        // place at the most right position
        drawRectangleOffset += windowWidth - totalWidth;
      }
      drawRectangle.left += drawRectangleOffset;
    }

    // main color for pieces
    bool enableColoring = true;
    COLORREF textColor = foregroundColor, pieceColor;
    if ( IsWindowEnabled( windowHandle ) == FALSE )
    {
      enableColoring = false;
      textColor = disabledForegroundColor;
    }

    SetTextColor( psEx.memoryContext, textColor );

    // if there is checkbox, RECT will be modified to appropriate size
    DrawCheckbox( psEx.memoryContext, drawRectangle, outerStyle );

    // paint pieces separately
    for ( const TextPiece tp : pieces )
    {
      if ( tp.size.cx > 0 )
      {
        pieceColor = textColor;
        if ( tp.message == MAXDWORD )
        {
          SelectObject( psEx.memoryContext, textFont );
        }
        else
        {
          if ( enableColoring )
          {
            if ( tp.state == STATE::HOVER )
            {
              pieceColor = hoverColor;
            }
            else if ( tp.state == STATE::CLICK )
            {
              pieceColor = clickColor;
            }
          }
          SelectObject( psEx.memoryContext, textUnderlinedFont );
        }
        SetTextColor( psEx.memoryContext, pieceColor );
        DrawText( psEx.memoryContext, tp.text.c_str(), -1, &drawRectangle, outerStyle );
        drawRectangle.left += tp.size.cx;
      }
    }
    Postpaint( psEx );
  }
  return 0;
}

int CompositeLinkControl::GetPieceByPosition( int xPosition ) const
{
  LONG offsetFromStart = drawRectangleOffset + CheckboxSize();
  for ( size_t index = 0; index < pieces.size(); ++index )
  {
    if ( xPosition > offsetFromStart && xPosition < ( offsetFromStart + pieces[ index ].size.cx ))
    {
      return index;
    }
    offsetFromStart += pieces[ index ].size.cx;
  }
  return -1;
}

LRESULT CompositeLinkControl::OnMouseLeave()
{
  SetState( STATE::NORMAL, -1 );
  return 0;
}

LRESULT CompositeLinkControl::OnMouseMove( WPARAM, LPARAM lParam )
{
  int overPiece = GetPieceByPosition( GET_X_LPARAM( lParam ));
  if ( overPiece == -1 )
  {
    OnMouseLeave();
  }
  else if ( pieces[ overPiece ].message == MAXDWORD )
  {
    // no link here, discard any states
    SetState( STATE::NORMAL, overPiece );
  }
  else
  {
    if ( previousCursor == NULL && currentState != STATE::CLICK && overPiece != activePiece )
    {
      mouseEvent.cbSize = sizeof( TRACKMOUSEEVENT );
      mouseEvent.dwFlags = TME_LEAVE;
      mouseEvent.hwndTrack = windowHandle;
      TrackMouseEvent( &mouseEvent );
      SetState( STATE::HOVER, overPiece );
    }
  }
  return 0;
}

void CompositeLinkControl::SetState( STATE newState, int overPiece )
{
  if ( newState != currentState || overPiece != activePiece )
  {
    // reset all exept new one
    bool redraw = false;
    for ( size_t index = 0; index < pieces.size(); ++index )
    {
      if ( index == ( size_t )overPiece )
      {
        if ( pieces[ index ].state != newState )
        {
          pieces[ index ].state = newState;
          redraw = true;
        }
      }
      else
      {
        if ( pieces[ index ].state != STATE::NORMAL )
        {
          pieces[ index ].state = STATE::NORMAL;
          redraw = true;
        }
      }
    }

    activePiece = overPiece;

    // something changed?
    // use original procedure to set the state
    LinkControl::SetState( newState );
  }
}

LRESULT CompositeLinkControl::OnLeftButtonDown( WPARAM /*wParam*/, LPARAM lParam )
{
  int pieceClicked = GetPieceByPosition( GET_X_LPARAM( lParam ));
  SetState( STATE::CLICK, pieceClicked );
  return (LRESULT) 1;
}

LRESULT CompositeLinkControl::OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam )
{
  return OnLeftButtonDown( wParam, lParam );
}

LRESULT CompositeLinkControl::OnLeftButtonUp( WPARAM, LPARAM lParam )
{
  int pieceClicked = GetPieceByPosition( GET_X_LPARAM( lParam ));
  if ( pieceClicked > -1 )
  {
    if ( pieces[ pieceClicked ].message != MAXDWORD )
    {
      if ( pieces[ pieceClicked ].state == STATE::CLICK )
      {
        SetState( STATE::NORMAL, pieceClicked );
        HWND parent = GetParent( windowHandle );
        if ( parent != NULL )
        {
          PostMessage( parent, WM_COMMAND, WORD( GetDlgCtrlID( windowHandle )), pieces[ pieceClicked ].message );
        }
      }
      else
      {
        SetState( STATE::NORMAL, pieceClicked );
      }
    }
    else
    {
      // click on non active area
      CheckboxClick();
    }
  }
  else
  {
    // click on non active area
    CheckboxClick();
  }
  return (LRESULT) 1;
}

LRESULT CompositeLinkControl::OnEraseBackground( HDC dc )
{
  DestroyBackground();
  if ( pieces.size() == 0 )
  {
    return LinkControl::OnEraseBackground( dc );
  }
  else
  {
    return ( LRESULT ) TRUE;
  }
}