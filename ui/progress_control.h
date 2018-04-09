// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/base_control.h"

class ProgressControl : public BaseControl
{
public:
  ProgressControl();
  virtual ~ProgressControl();

  virtual LRESULT OnPaint() override;
  virtual LRESULT OnEraseBackground( HDC ) override;
  void Step( DWORD total, DWORD current );
  void GetSteps( DWORD& total, DWORD& current );

protected:

  // for normal bar
  DWORD totalPosition;
  DWORD currentPosition;

  // for marque bar
  LONG step;
  // slices in marque mode
  DWORD slices;
};

