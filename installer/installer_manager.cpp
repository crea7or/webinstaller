// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"

#include "windows/abstract_dialog.h"
#include "windows/windows_utils.h"
#include "installer/installer_constants.h"
#include "installer/installer_manager.h"
#include "installer/resource.h"
#include "installer/system_utils.h"

InstallerManager::InstallerManager()
{
  activeWindow = NULL;
  installerFont = NULL;
  installerRunThreadHandle = NULL;

  // add some options
  // options controlId should be greater than max ID of any control ID in resources.h
  options.push_back( { L"Install Amigo browser", L"license agreement", L"https://imgur.com/gallery/L9N78", 4000, false } );
  options.push_back( { L"Burn my computer with fire", L"privacy policy", L"https://imgur.com/gallery/8mTIGm4", 4001, false } );
  options.push_back( { L"Third funny option", L"", L"", 4002, false } );
};

InstallerManager::~InstallerManager()
{
  if ( InstallerRunThread != NULL )
  {
    WaitForSingleObject( installerRunThreadHandle, INFINITE );
    CloseHandle( installerRunThreadHandle );
  }
  if ( installerFont != NULL )
  {
    WindowsUtils::UnloadFontFromResource( installerFont );
  }
}

InstallerManager& InstallerManager::Get()
{
  // Guaranteed to be destroyed. Instantiated on first use.
  static InstallerManager instance;
  return instance;
}

void InstallerManager::InitializeWithInstance( HINSTANCE appInstance )
{
  // before loading font
  if ( installerFont == NULL )
  {
    installerFont = WindowsUtils::LoadFontFromResource( appInstance, IDR_WEBINSTALLER_FONT );
  }
}

HWND InstallerManager::GetActiveWindow() const
{
  return activeWindow;
}

void InstallerManager::SetActiveWindow( HWND handle )
{
  activeWindow = handle;
}

void InstallerManager::InstallerStart()
{
  if ( installerRunThreadHandle == NULL )
  {
    installerRunThreadHandle = CreateThread( NULL, 0, InstallerManager::InstallerRunThread, NULL, 0, NULL );
  }
}

DWORD WINAPI InstallerManager::InstallerRunThread( LPVOID )
{
  InstallerManager& manager = InstallerManager::Get();
  HWND window = manager.GetActiveWindow();
  std::wstring infoText( L"Downloading..." );
  SendMessage( window, SET_PROGRESS_TEXT, 0, ( LPARAM ) infoText.c_str());

  for ( uint32_t download = 0; download < 50; ++download )
  {
    if ( window != NULL )
    {
      PostMessage( window, SET_PROGRESS_BAR, ( WPARAM ) 50, ( LPARAM ) download );
    }
    Sleep( 100 );
  }

  infoText = L"Unpacking and launching...";
  SendMessage( window, SET_PROGRESS_TEXT, 0, ( LPARAM ) infoText.c_str());
  PostMessage( window, SET_PROGRESS_BAR, ( WPARAM ) 0, ( LPARAM ) 0 );

  for ( uint32_t install = 0; install < 50; ++install )
  {
    Sleep( 100 );
  }

  PostMessage( window, SETUP_COMPLETE, ( WPARAM ) 0, ( LPARAM ) 0 );

  return 0;
}

void InstallerManager::SetOptionSelected( const UINT controlId, const bool state )
{
  auto it = std::find_if( options.begin(), options.end(), [ &controlId ]( const InstallOption& obj ) { return obj.controlId == controlId; } );
  if ( it != options.end())
  {
    it->state = state;
  }
}

bool InstallerManager::GetOptionSelected( const UINT controlId ) const
{
  auto it = std::find_if( options.begin(), options.end(), [ &controlId ]( const InstallOption& obj ) { return obj.controlId == controlId; } );
  if ( it != options.end())
  {
    return it->state;
  }
  else
  {
    return false;
  }
}

std::wstring InstallerManager::GetOptionLinkUrl( const UINT controlId ) const
{
  std::wstring result;
  auto it = std::find_if( options.begin(), options.end(), [ &controlId ]( const InstallOption& obj ) { return obj.controlId == controlId; } );
  if ( it != options.end())
  {
    result = it->linkUrl;
  }
  return result;
}

