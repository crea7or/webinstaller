// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/static_control.h"

#include <vector>

struct StaticLayer
{
  std::wstring text;
  COLORREF color;
};

class LayeredStaticControl : public StaticControl
{
public:
  LayeredStaticControl();
  virtual ~LayeredStaticControl();

  virtual LRESULT OnPaint() override;

  void AddLayer( const std::wstring& text, const COLORREF color )
  {
    layers.push_back( StaticLayer( { text, color } ));
  }

protected:

  std::vector< StaticLayer > layers;
};

