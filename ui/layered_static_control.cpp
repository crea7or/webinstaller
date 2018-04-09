// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/layered_static_control.h"

LayeredStaticControl::LayeredStaticControl()
{}

LayeredStaticControl::~LayeredStaticControl()
{}

LRESULT LayeredStaticControl::OnPaint()
{
  PAINTSTRUCT ps;
  BeginPaint( windowHandle, &ps );
  RECT windowRectangle;
  GetClientRect( windowHandle, &windowRectangle );
  int oldMode = SetBkMode( ps.hdc, TRANSPARENT );
  HFONT oldFont = ( HFONT )SelectObject( ps.hdc, textFont );

  for ( const StaticLayer& layer : layers )
  {
    SetTextColor( ps.hdc, layer.color );
    DrawText( ps.hdc, layer.text.c_str(), -1, &windowRectangle, GetControlStyles( DT_EDITCONTROL | DT_WORDBREAK | DT_TOP ));
  }

  SelectObject( ps.hdc, oldFont );
  SetBkMode( ps.hdc, oldMode );
  EndPaint( windowHandle, &ps );

  return 0;
}

