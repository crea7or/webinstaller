// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/base_control.h"
#include "windows/windows_utils.h"

struct PAINTSTRUCTEX : public PAINTSTRUCT
{
  HFONT previousFont;
  HBITMAP previousMemoryBitmap;
  int previousBkMode;
  HBITMAP memoryBitmap;
  HDC memoryContext;
  WindowsUtils::RectHolder windowRectangle;
};

class StaticControl : public BaseControl
{
public:
  StaticControl();
  virtual ~StaticControl();

  enum STATE : uint8_t
  {
    NORMAL = 0,
    HOVER = 1,
    CLICK = 2,
  };

  virtual LRESULT OnPaint() override;
  virtual LRESULT OnEraseBackground( HDC deviceContext ) override;

  void SetHoverColor( COLORREF color )
  {
    hoverColor = color;
  }
  COLORREF GetHoverColor() const
  {
    return hoverColor;
  }

  void SetClickColor( COLORREF color )
  {
    clickColor = color;
  }
  COLORREF GetClickColor() const
  {
    return clickColor;
  }

  virtual void SetText( const std::wstring& text ) override;
  virtual std::wstring GetText() override;

protected:

  DWORD GetControlStyles( const DWORD initialStyle ) const;
  COLORREF GetControlTextColor() const;

  void Prepaint( PAINTSTRUCTEX& paintStruct );
  void Postpaint( PAINTSTRUCTEX& paintStruct );
  void DestroyBackground();

  virtual void SetState( STATE newState )
  {
    currentState = newState;
  }

  STATE currentState;

  COLORREF hoverColor;
  COLORREF clickColor;

  // in memory background copy
  HBITMAP clearBackgroundBitmap;
  HBITMAP oldBackgroundBitmap;
  HDC backgroundContext;
  // in memory background copy
};
