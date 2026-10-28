// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "tests/test_framework.h"
#include "installer/installer_manager.h"

namespace
{
  InstallerManager& Manager()
  {
    return InstallerManager::Get();
  }

  // installer manager is a singleton, tests restore option states they change
  void ResetOptions()
  {
    for ( const InstallOption& option : Manager().GetOptions())
    {
      Manager().SetOptionSelected( option.controlId, false );
    }
  }
}

TEST( InstallerManager, GetReturnsSameInstance )
{
  EXPECT_TRUE( &InstallerManager::Get() == &InstallerManager::Get());
}

TEST( InstallerManager, DefaultOptions )
{
  const std::vector< InstallOption >& options = Manager().GetOptions();
  ASSERT_EQ( options.size(), 3u );
  EXPECT_EQ( options[ 0 ].controlId, 4000u );
  EXPECT_EQ( options[ 1 ].controlId, 4001u );
  EXPECT_EQ( options[ 2 ].controlId, 4002u );
  for ( const InstallOption& option : options )
  {
    EXPECT_FALSE( option.state );
    EXPECT_FALSE( option.name.empty());
    // option ids must not collide with dialog resource ids
    EXPECT_TRUE( option.controlId > 1009u );
  }
}

TEST( InstallerManager, SetAndGetOptionSelected )
{
  ResetOptions();
  Manager().SetOptionSelected( 4001, true );
  EXPECT_FALSE( Manager().GetOptionSelected( 4000 ));
  EXPECT_TRUE( Manager().GetOptionSelected( 4001 ));
  EXPECT_FALSE( Manager().GetOptionSelected( 4002 ));
  EXPECT_TRUE( Manager().GetOptions()[ 1 ].state );

  Manager().SetOptionSelected( 4001, false );
  EXPECT_FALSE( Manager().GetOptionSelected( 4001 ));
  ResetOptions();
}

TEST( InstallerManager, UnknownOptionIsIgnored )
{
  ResetOptions();
  Manager().SetOptionSelected( 12345, true );
  EXPECT_FALSE( Manager().GetOptionSelected( 12345 ));
  for ( const InstallOption& option : Manager().GetOptions())
  {
    EXPECT_FALSE( option.state );
  }
}

TEST( InstallerManager, OptionLinkUrl )
{
  EXPECT_EQ( Manager().GetOptionLinkUrl( 4000 ), std::wstring( L"https://imgur.com/gallery/L9N78" ));
  EXPECT_EQ( Manager().GetOptionLinkUrl( 4001 ), std::wstring( L"https://imgur.com/gallery/8mTIGm4" ));
  // option without link
  EXPECT_TRUE( Manager().GetOptionLinkUrl( 4002 ).empty());
  // unknown option
  EXPECT_TRUE( Manager().GetOptionLinkUrl( 1 ).empty());
}

TEST( InstallerManager, ActiveWindow )
{
  HWND previous = Manager().GetActiveWindow();
  Manager().SetActiveWindow(( HWND ) 0x1234 );
  EXPECT_TRUE( Manager().GetActiveWindow() == ( HWND ) 0x1234 );
  Manager().SetActiveWindow( previous );
}

// install thread

namespace
{
  struct InstallEvents
  {
    std::vector< std::wstring > texts;
    std::vector< std::pair< WPARAM, LPARAM >> progress;
    int completed = 0;
  };

  InstallEvents* installEvents = NULL;

  LRESULT CALLBACK InstallReceiverProc( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
  {
    if ( installEvents != NULL )
    {
      switch ( message )
      {
        case SET_PROGRESS_TEXT:
          // sent synchronously, text pointer is valid only during the call
          installEvents->texts.push_back(( const wchar_t* ) lParam );
          return 0;
        case SET_PROGRESS_BAR:
          installEvents->progress.push_back( std::make_pair( wParam, lParam ));
          return 0;
        case SETUP_COMPLETE:
          ++installEvents->completed;
          return 0;
      }
    }
    return DefWindowProc( window, message, wParam, lParam );
  }

  // processes sent and posted messages for the window until timeout or condition
  template < typename Condition >
  void PumpUntil( HWND window, DWORD timeout, Condition condition )
  {
    DWORD start = GetTickCount();
    while ( !condition() && GetTickCount() - start < timeout )
    {
      MsgWaitForMultipleObjects( 0, NULL, FALSE, 20, QS_ALLINPUT );
      MSG msg;
      while ( PeekMessage( &msg, window, 0, 0, PM_REMOVE ))
      {
        DispatchMessage( &msg );
      }
    }
  }
}

// install thread can be started once per process, so start and cancel are one test
TEST( InstallerManager, InstallThreadReportsProgressAndStopsOnCancel )
{
  WNDCLASSEX windowClass = { sizeof( WNDCLASSEX ) };
  windowClass.lpfnWndProc = InstallReceiverProc;
  windowClass.hInstance = GetModuleHandle( NULL );
  windowClass.lpszClassName = L"WebInstallerTestInstallReceiver";
  RegisterClassEx( &windowClass );
  HWND receiver = CreateWindowEx( 0, windowClass.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle( NULL ), NULL );
  ASSERT_TRUE( receiver != NULL );

  InstallEvents events;
  installEvents = &events;
  HWND previous = Manager().GetActiveWindow();
  Manager().SetActiveWindow( receiver );

  Manager().InstallerStart();
  // second start while running must not start another thread
  Manager().InstallerStart();

  PumpUntil( receiver, 3000, [ &events ]() { return events.progress.size() >= 3; } );

  ASSERT_EQ( events.texts.size(), 1u );
  EXPECT_EQ( events.texts[ 0 ], std::wstring( L"Downloading..." ));
  ASSERT_TRUE( events.progress.size() >= 3 );
  // reports completed steps out of 50, starting from 1
  for ( size_t index = 0; index < events.progress.size(); ++index )
  {
    EXPECT_EQ( events.progress[ index ].first, ( WPARAM ) 50 );
    EXPECT_EQ( events.progress[ index ].second, ( LPARAM )( index + 1 ));
  }

  Manager().InstallerCancel();
  // allow one report that may already be in flight
  PumpUntil( receiver, 300, []() { return false; } );
  size_t progressAfterCancel = events.progress.size();
  PumpUntil( receiver, 500, []() { return false; } );

  EXPECT_EQ( events.progress.size(), progressAfterCancel );
  EXPECT_EQ( events.completed, 0 );
  EXPECT_EQ( events.texts.size(), 1u );
  // nowhere near the full download
  EXPECT_TRUE( events.progress.size() < 50 );

  installEvents = NULL;
  Manager().SetActiveWindow( previous );
  DestroyWindow( receiver );
}
