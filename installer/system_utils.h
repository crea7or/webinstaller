// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <string>

namespace SystemUtils
{
  // HiDpi functions
  UINT GetDpiForWindowUser32( HWND hwnd );
  UINT GetDpiForMonitorShcore( HWND window );
};

