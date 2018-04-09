// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/composite_link_control.h"

class CompositeCheckboxControl : public CompositeLinkControl
{
public:

  enum CHECK_SYMBOL : wchar_t
  {
    UNCHECKED = L'<',
    CHECKED = L'>',
    BACKGROUND = L'=',
  };

  CompositeCheckboxControl();
  virtual ~CompositeCheckboxControl();

  virtual void SetFont( LOGFONT* fontStruct ) override;

  virtual LONG CheckboxSize() const override
  {
    return checkboxSize;
  }

  virtual LONG CalculateCheckboxSize( const HDC deviceContext ) override;
  virtual LONG DrawCheckbox( const HDC deviceContext, RECT& drawRectangle, const UINT outerStyle ) override;
  virtual void CheckboxClick();

  virtual void SetCheck( const bool state );
  virtual bool GetCheck() const
  {
    return checkState;
  }

  void SetCheckBackColor( const COLORREF color )
  {
    checkBackColor = color;
  }
  void SetCheckFrontColor( const COLORREF color )
  {
    checkFrontColor = color;
  }
protected:

  void DestroyCheckboxFont();

  // font for checkbox - symbols {|} are checkbox states
  // { - empty checkbox frame
  // | - filled checkbox frame
  // } - check with checkbox frame
  HFONT checkboxFont;

  // size of checkbox
  LONG checkboxSize;

  // checkbox state
  bool checkState;

  // checkbox background color
  COLORREF checkBackColor;
  // checkbox foreground color
  COLORREF checkFrontColor;

};

