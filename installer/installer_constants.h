// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include <windows.h>

// undef for plain window background
#define GRADIENT_BG

namespace Installer
{
  const wchar_t eMsgWindowTitle[] = L"Web Installer";
  const wchar_t eMsgCancelMessage[] = L"Cancell install process?";

  const wchar_t eApiInstallerEula[] = L"https://habrahabr.ru/users/crea7or/";
  const wchar_t eApiInstallerPolicy[] = L"http://crea7or.spb.ru/?installer";

#ifdef GRADIENT_BG
  const COLORREF mainTextColor = RGB( 0x0f, 0x0f, 0x0e );
  const COLORREF progressBackgroundColor = RGB( 0x19, 0xa1, 0x5f );
  const COLORREF progressForegroundColor = RGB( 0x28, 0xff, 0x96 );
  const COLORREF grayColor = RGB( 0xe0, 0xe0, 0xe0 );
#else
  const COLORREF mainTextColor = RGB( 0x16, 0x22, 0x28 );
  const COLORREF progressBackgroundColor = RGB( 0x19, 0xa1, 0x5f );
  const COLORREF progressForegroundColor = RGB( 0x28, 0xff, 0x96 );
  const COLORREF grayColor = RGB( 0xf5, 0xf6, 0xf6 );
#endif

};
