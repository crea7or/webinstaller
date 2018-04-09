// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/link_control.h"

class RectButtonControl : public LinkControl
{
public:
  RectButtonControl();
  virtual ~RectButtonControl();

  virtual LRESULT OnPaint() override;
  virtual LRESULT OnLeftButtonDown( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnLeftButtonUp( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam ) override;

  void SetTextShadowColor( COLORREF color )
  {
    textShadowColor = color;
  }

  void SetButtonNormalColor( COLORREF color )
  {
    buttonNormalColor = color;
  }
  void SetButtonHoverColor( COLORREF color )
  {
    buttonHoverColor = color;
  }
  void SetButtonClickColor( COLORREF color )
  {
    buttonClickColor = color;
  }

protected:

  COLORREF textShadowColor;
  COLORREF buttonNormalColor;
  COLORREF buttonHoverColor;
  COLORREF buttonClickColor;

};

