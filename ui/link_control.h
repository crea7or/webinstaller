// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/static_control.h"

class LinkControl : public StaticControl
{
public:
  LinkControl();
  virtual ~LinkControl();

  virtual LRESULT OnMouseMove( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnMouseLeave() override;
  virtual LRESULT OnLeftButtonDown( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnLeftButtonUp( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnLeftButtonDoubleDown( WPARAM wParam, LPARAM lParam ) override;

protected:

  virtual void SetState( STATE newState ) override;

  TRACKMOUSEEVENT mouseEvent;
  COLORREF savedTextColor;
  HCURSOR previousCursor;
  HCURSOR linkCursor;
};
