// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/composite_checkbox_control.h"

#include <windowsx.h>

CompositeCheckboxControl::CompositeCheckboxControl()
{
  checkboxFont = NULL;
  checkboxSize = 0;
  checkState = false;
  checkBackColor = GetBackgroundColor();
  checkFrontColor = GetForegroundColor();
}

CompositeCheckboxControl::~CompositeCheckboxControl()
{
  DestroyCheckboxFont();
}

void CompositeCheckboxControl::DestroyCheckboxFont()
{
  if ( checkboxFont != NULL )
  {
    DeleteObject( checkboxFont );
    checkboxFont = NULL;
  }
}

void CompositeCheckboxControl::SetFont( LOGFONT* fontStruct )
{
  LONG backupHeight = fontStruct->lfHeight;
  fontStruct->lfHeight -= 4;
  BaseControl::SetFont( fontStruct );
  DestroyCheckboxFont();
  checkboxFont = textFont;
  // set to null ot avoid font release by base class while new font will be set
  textFont = NULL;
  fontStruct->lfHeight = backupHeight;
  CompositeLinkControl::SetFont( fontStruct );
}

LONG CompositeCheckboxControl::CalculateCheckboxSize( const HDC deviceContext )
{
  checkboxSize = 0;
  HFONT oldFont = (HFONT) SelectObject( deviceContext, checkboxFont );
  std::wstring checkbox( 1, CHECK_SYMBOL::CHECKED );
  // appeand one space for checkbox
  checkbox.append( L"  " );
  RECT textRect = { 0 };
  if ( DrawText( deviceContext, checkbox.c_str(), checkbox.size(), &textRect, DT_CALCRECT ) > 0 )
  {
    checkboxSize = textRect.right - textRect.left;
  }
  SelectObject( deviceContext, oldFont );
  return checkboxSize;
}

LONG CompositeCheckboxControl::DrawCheckbox( const HDC deviceContext, RECT& drawRectangle, const UINT outerStyle )
{
  if ( checkboxSize > 0 )
  {
    // draw checkbox background
    HFONT oldFont = (HFONT) SelectObject( deviceContext, checkboxFont );
    SetTextColor( deviceContext, checkBackColor );

    std::wstring checkboxBackground( 1, CHECK_SYMBOL::BACKGROUND );
    DrawText( deviceContext, checkboxBackground.c_str(), checkboxBackground.size(), &drawRectangle, outerStyle );

    SetTextColor( deviceContext, checkFrontColor );

    // draw checkbox
    if ( checkState )
    {
      std::wstring checkboxChecked( 1, CHECK_SYMBOL::CHECKED );
      DrawText( deviceContext, checkboxChecked.c_str(), checkboxChecked.size(), &drawRectangle, outerStyle );
    }
    else
    {
      std::wstring checkboxUnchecked( 1, CHECK_SYMBOL::UNCHECKED );
      DrawText( deviceContext, checkboxUnchecked.c_str(), checkboxUnchecked.size(), &drawRectangle, outerStyle );
    }
    // restore fornt in context
    SelectObject( deviceContext, oldFont );
    // increate indent for further drawing
    drawRectangle.left += checkboxSize;
  }
  return checkboxSize;
}

void CompositeCheckboxControl::CheckboxClick()
{
  SetCheck( !GetCheck());
}

void CompositeCheckboxControl::SetCheck( const bool state )
{
  checkState = state;
  if ( windowHandle != NULL )
  {
    RedrawControl();
  }
}
