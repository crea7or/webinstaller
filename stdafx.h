// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#pragma warning( disable : 4091 )

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>


#include <stdint.h>

#include <string>
#include <vector>
#include <algorithm>

// gradient fill functions in basedialog
#pragma comment( lib, "msimg32.lib" )