// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/link_control.h"

#include <string>
#include <vector>

struct TextPiece
{
  // initially set
  TextPiece( const std::wstring& _text,const UINT _message = MAXDWORD ) :
    text( _text ), size({ 0 }), state( StaticControl::STATE::NORMAL ), message( _message ) {}
  std::wstring text;
  // will be set later by OnPaint
  SIZE size;
  // piece state
  StaticControl::STATE state;
  // message to be sent on click
  UINT message;
};

class CompositeLinkControl : public LinkControl
{
public:
  CompositeLinkControl();
  virtual ~CompositeLinkControl();

  bool AttachWithText( const HWND ownerWindow, const int windowId );
  bool AttachWithText( const HWND controlWindow );

  void AddPiece( const std::wstring& _text, const UINT _message = MAXDWORD )
  {
    pieces.push_back( TextPiece( _text, _message ));
  }

  virtual LRESULT OnPaint() override;
  virtual void SetFont( LOGFONT* fontStruct ) override;

  virtual LRESULT OnMouseLeave() override;
  virtual LRESULT OnMouseMove( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnLeftButtonDown( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnLeftButtonUp( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam ) override;

  virtual LONG CheckboxSize() const { return 0; }
  virtual LONG CalculateCheckboxSize( const HDC ) { return 0; }
  virtual LONG DrawCheckbox( const HDC , RECT& , const UINT ) { return 0; }
  virtual void CheckboxClick() {}
  virtual LRESULT OnEraseBackground( HDC deviceContext ) override;

protected:

  void SetState( STATE newState, int overPiece );
  int GetPieceByPosition( int xPosition ) const;
  void DestroyUnderlinedFont();

  std::vector< TextPiece > pieces;

  // piece index that have some highlighting or -1
  LONG activePiece;
  // offset used to draw the text (to handle aligning of text)
  LONG drawRectangleOffset;
  // font with underline style
  HFONT textUnderlinedFont;

};

