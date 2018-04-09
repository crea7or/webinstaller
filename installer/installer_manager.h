// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "windows/abstract_dialog.h"
#include "windows/windows_utils.h"
#include "installer/installer_constants.h"
#include "installer/resource.h"
#include "installer/system_utils.h"

class InstallOption
{
public:
  InstallOption( const std::wstring& _name, const std::wstring& _linkText, const std::wstring& _linkUrl, const UINT _controlId, const bool _state )
    : name( _name ), linkText( _linkText ), linkUrl( _linkUrl ), controlId( _controlId ), state( _state ) {};
  std::wstring name;
  std::wstring linkText;
  std::wstring linkUrl;
  UINT controlId;
  bool state;
};

// singleton for web installer
class InstallerManager
{
public:
  static InstallerManager& Get();

  // load custom font etc.
  void InitializeWithInstance( HINSTANCE appInstance );

  // start install process (download, unpack, launch ...)
  void InstallerStart();

  HWND GetActiveWindow() const;
  void SetActiveWindow( HWND handle );

  void SetOptionSelected( const UINT controlId, const bool state );
  bool GetOptionSelected( const UINT controlId ) const;
  std::wstring GetOptionLinkUrl( const UINT controlId ) const;

  static DWORD WINAPI InstallerRunThread( LPVOID );

  const std::vector< InstallOption >& GetOptions() const
  {
    return options;
  }

private:

  // active window handle
  volatile HWND activeWindow;

  // installer font
  volatile HANDLE installerFont;

  // install thread handle
  volatile HANDLE installerRunThreadHandle;

  InstallerManager();
  ~InstallerManager();

  std::vector< InstallOption > options;

public:

  InstallerManager( InstallerManager const& ) = delete;
  void operator=( InstallerManager const& ) = delete;
};

