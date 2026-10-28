// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// dialog behaviour tests: message routing, lifetime, template based layout and main dialog state
// dialogs are created from the installer resources linked into the test executable
// and are never shown, so no painting happens

#include "stdafx.h"
#include "tests/test_framework.h"
#include "windows/abstract_dialog.h"
#include "windows/main_dialog.h"
#include "installer/installer_manager.h"
#include "installer/resource.h"

namespace
{
  struct RecordedMessage
  {
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
  };

  // records which handlers DialogFunction dispatches to
  class RecordingDialog : public AbstractDialog
  {
  public:
    RecordingDialog() : AbstractDialog( GetModuleHandle( NULL ), IDD_DIALOG_MAIN )
    {}

    virtual LRESULT OnInitDialog( HWND window ) override
    {
      ++initCalls;
      return AbstractDialog::OnInitDialog( window );
    }
    virtual LRESULT OnUserMessage( UINT message, WPARAM wParam, LPARAM lParam ) override
    {
      userMessages.push_back( { message, wParam, lParam } );
      return 0;
    }
    virtual LRESULT OnCommand( WPARAM wParam, LPARAM lParam ) override
    {
      commands.push_back( { WM_COMMAND, wParam, lParam } );
      return 0;
    }
    virtual LRESULT OnTimer( WPARAM, LPARAM ) override
    {
      ++timerCalls;
      return 0;
    }
    virtual LRESULT OnClose() override
    {
      ++closeCalls;
      return 0;
    }

    int initCalls = 0;
    int timerCalls = 0;
    int closeCalls = 0;
    std::vector< RecordedMessage > userMessages;
    std::vector< RecordedMessage > commands;
  };

  // exposes main dialog controls
  class TestableMainDialog : public MainDialog
  {
  public:
    TestableMainDialog() : MainDialog( GetModuleHandle( NULL ))
    {}

    using MainDialog::progressControl;
    using MainDialog::infoTextControl;
    using MainDialog::installButtonControl;
    using MainDialog::checkboxes;
  };

  // removes WM_QUIT posted by DestroyDialog( true ), returns true if there was one
  bool TakeQuitMessage()
  {
    MSG msg;
    return PeekMessage( &msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE ) != FALSE;
  }

  RECT ChildClientRect( HWND dialog, UINT id )
  {
    RECT rect = { 0 };
    GetWindowRect( GetDlgItem( dialog, id ), &rect );
    MapWindowPoints( HWND_DESKTOP, dialog, ( LPPOINT ) &rect, 2 );
    return rect;
  }

  bool IsVisibleStyle( HWND window )
  {
    return ( GetWindowLong( window, GWL_STYLE ) & WS_VISIBLE ) != 0;
  }

  // installer manager is a singleton, main dialog registers itself as its active window
  struct ActiveWindowGuard
  {
    ActiveWindowGuard() : previous( InstallerManager::Get().GetActiveWindow()) {}
    ~ActiveWindowGuard() { InstallerManager::Get().SetActiveWindow( previous ); }
    HWND previous;
  };
}

// AbstractDialog

TEST( AbstractDialog, CreateInitializesDialog )
{
  RecordingDialog dialog;
  EXPECT_TRUE( dialog.GetDialogHandle() == NULL );
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();
  EXPECT_TRUE( IsWindow( handle ) != FALSE );
  EXPECT_EQ( dialog.initCalls, 1 );
  EXPECT_TRUE(( AbstractDialog* ) GetWindowLongPtr( handle, GWLP_USERDATA ) == &dialog );
  EXPECT_FALSE( IsVisibleStyle( handle ));
}

TEST( AbstractDialog, UserMessagesAreRoutedFromWmUser )
{
  RecordingDialog dialog;
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();

  SendMessage( handle, WM_USER - 1, 1, 2 );
  SendMessage( handle, WM_USER, 3, 4 );
  SendMessage( handle, SET_PROGRESS_TEXT, 5, 6 );
  SendMessage( handle, WM_APP + 7, 8, 9 );

  ASSERT_EQ( dialog.userMessages.size(), 3u );
  EXPECT_EQ( dialog.userMessages[ 0 ].message, ( UINT ) WM_USER );
  EXPECT_EQ( dialog.userMessages[ 0 ].wParam, ( WPARAM ) 3 );
  EXPECT_EQ( dialog.userMessages[ 0 ].lParam, ( LPARAM ) 4 );
  EXPECT_EQ( dialog.userMessages[ 1 ].message, ( UINT ) SET_PROGRESS_TEXT );
  EXPECT_EQ( dialog.userMessages[ 2 ].message, ( UINT )( WM_APP + 7 ));
}

TEST( AbstractDialog, CommandAndCloseAreRouted )
{
  RecordingDialog dialog;
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();

  SendMessage( handle, WM_COMMAND, MAKEWPARAM( IDC_SETUP_BUTTON, 0 ), 77 );
  ASSERT_EQ( dialog.commands.size(), 1u );
  EXPECT_EQ( LOWORD( dialog.commands[ 0 ].wParam ), IDC_SETUP_BUTTON );
  EXPECT_EQ( dialog.commands[ 0 ].lParam, ( LPARAM ) 77 );

  SendMessage( handle, WM_CLOSE, 0, 0 );
  EXPECT_EQ( dialog.closeCalls, 1 );
  // closing is up to OnClose
  EXPECT_TRUE( IsWindow( handle ) != FALSE );
}

TEST( AbstractDialog, OnlyDialogTimerIsRouted )
{
  RecordingDialog dialog;
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();

  // timer id equal to the dialog handle is the dialog's own timer
  SendMessage( handle, WM_TIMER, ( WPARAM ) handle, 0 );
  SendMessage( handle, WM_TIMER, ( WPARAM ) 1, 0 );
  EXPECT_EQ( dialog.timerCalls, 1 );
}

TEST( AbstractDialog, DestroyDialogIsSafeToRepeat )
{
  RecordingDialog dialog;
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();

  dialog.DestroyDialog( false );
  EXPECT_TRUE( dialog.GetDialogHandle() == NULL );
  EXPECT_FALSE( IsWindow( handle ));
  EXPECT_FALSE( TakeQuitMessage());

  // already destroyed, so it must not post quit either
  dialog.DestroyDialog( true );
  EXPECT_FALSE( TakeQuitMessage());
}

TEST( AbstractDialog, DestroyDialogWithQuitPostsQuit )
{
  RecordingDialog dialog;
  ASSERT_TRUE( dialog.Create());
  dialog.DestroyDialog( true );
  EXPECT_TRUE( TakeQuitMessage());
}

TEST( AbstractDialog, DestructorDestroysWindow )
{
  HWND handle = NULL;
  {
    RecordingDialog dialog;
    ASSERT_TRUE( dialog.Create());
    handle = dialog.GetDialogHandle();
  }
  EXPECT_FALSE( IsWindow( handle ));
}

TEST( AbstractDialog, LayoutIsConvertedFromDialogUnits )
{
  RecordingDialog dialog;
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();

  // DLU are converted to 96 DPI pixels: x * 3 / 2, y * 13 / 8 (truncated),
  // then scaled to the window DPI (the test executable embeds the installer manifest, so it is DPI aware)
  LONG dpi = ( LONG ) WindowsUtils::GetWindowDpi( handle );
  auto x = [ dpi ]( LONG dlu ) { return WindowsUtils::MultiplyThenDivide( dlu * 3 / 2, dpi, 96 ); };
  auto y = [ dpi ]( LONG dlu ) { return WindowsUtils::MultiplyThenDivide( dlu * 13 / 8, dpi, 96 ); };

  // window is scaled in screen coordinates, so allow one pixel of rounding
  RECT window = { 0 };
  GetWindowRect( handle, &window );
  EXPECT_TRUE( abs(( window.right - window.left ) - WindowsUtils::MultiplyThenDivide( 450 * 3 / 2, dpi, 96 )) <= 1 );
  EXPECT_TRUE( abs(( window.bottom - window.top ) - WindowsUtils::MultiplyThenDivide( 264 * 13 / 8, dpi, 96 )) <= 1 );

  // IDC_PROGRESS is 24, 152, 400, 20 in the template
  RECT progress = ChildClientRect( handle, IDC_PROGRESS );
  EXPECT_EQ( progress.left, x( 24 ));
  EXPECT_EQ( progress.top, y( 152 ));
  EXPECT_EQ( progress.right, x( 24 + 400 ));
  EXPECT_EQ( progress.bottom, y( 152 + 20 ));

  // IDC_STATIC_CLOSE is 424, 4, 14, 20 in the template
  RECT close = ChildClientRect( handle, IDC_STATIC_CLOSE );
  EXPECT_EQ( close.left, x( 424 ));
  EXPECT_EQ( close.top, y( 4 ));
  EXPECT_EQ( close.right, x( 424 + 14 ));
  EXPECT_EQ( close.bottom, y( 4 + 20 ));
}

// MainDialog

TEST( MainDialog, CreatesOneCheckboxPerInstallOption )
{
  ActiveWindowGuard guard;
  TestableMainDialog dialog;
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();

  const std::vector< InstallOption >& options = InstallerManager::Get().GetOptions();
  ASSERT_EQ( dialog.checkboxes.size(), options.size());
  LONG previousTop = -1;
  for ( size_t index = 0; index < options.size(); ++index )
  {
    HWND checkbox = dialog.checkboxes[ index ]->GetWindow();
    ASSERT_TRUE( checkbox != NULL );
    EXPECT_EQ(( UINT ) GetDlgCtrlID( checkbox ), options[ index ].controlId );
    EXPECT_TRUE( GetParent( checkbox ) == handle );
    // option name becomes the first text piece
    EXPECT_EQ( dialog.checkboxes[ index ]->GetText(), options[ index ].name );

    // stacked from top to bottom
    RECT rect = ChildClientRect( handle, options[ index ].controlId );
    EXPECT_TRUE( rect.top > previousTop );
    previousTop = rect.top;
  }

  // placeholder only gives the position
  EXPECT_FALSE( IsVisibleStyle( GetDlgItem( handle, IDC_STATIC_CHECKBOX_PLACEHOLDER )));
  // first checkbox takes the placeholder position
  RECT placeholder = ChildClientRect( handle, IDC_STATIC_CHECKBOX_PLACEHOLDER );
  RECT first = ChildClientRect( handle, options[ 0 ].controlId );
  EXPECT_EQ( first.left, placeholder.left );
  EXPECT_EQ( first.top, placeholder.top );
}

TEST( MainDialog, CheckboxesReflectInstallerOptions )
{
  ActiveWindowGuard guard;
  InstallerManager& manager = InstallerManager::Get();
  manager.SetOptionSelected( 4000, true );
  {
    TestableMainDialog dialog;
    ASSERT_TRUE( dialog.Create());
    EXPECT_TRUE( dialog.checkboxes[ 0 ]->GetCheck());
    EXPECT_FALSE( dialog.checkboxes[ 1 ]->GetCheck());

    dialog.checkboxes[ 1 ]->SetCheck( true );
    EXPECT_TRUE( manager.GetOptionSelected( 4001 ));
  }
  manager.SetOptionSelected( 4000, false );
  manager.SetOptionSelected( 4001, false );
}

TEST( MainDialog, RegistersAsInstallerActiveWindow )
{
  ActiveWindowGuard guard;
  TestableMainDialog dialog;
  ASSERT_TRUE( dialog.Create());
  EXPECT_TRUE( InstallerManager::Get().GetActiveWindow() == dialog.GetDialogHandle());
}

TEST( MainDialog, StartsInInitialMode )
{
  ActiveWindowGuard guard;
  TestableMainDialog dialog;
  ASSERT_TRUE( dialog.Create());

  EXPECT_TRUE( IsVisibleStyle( dialog.installButtonControl.GetWindow()));
  EXPECT_FALSE( IsVisibleStyle( dialog.progressControl.GetWindow()));
  EXPECT_FALSE( IsVisibleStyle( dialog.infoTextControl.GetWindow()));
}

TEST( MainDialog, ProgressMessagesUpdateControls )
{
  ActiveWindowGuard guard;
  TestableMainDialog dialog;
  ASSERT_TRUE( dialog.Create());
  HWND handle = dialog.GetDialogHandle();

  SendMessage( handle, SET_PROGRESS_TEXT, 0, ( LPARAM ) L"Downloading..." );
  EXPECT_EQ( dialog.infoTextControl.GetText(), std::wstring( L"Downloading..." ));

  SendMessage( handle, SET_PROGRESS_BAR, 50, 20 );
  DWORD total = 0, current = 0;
  dialog.progressControl.GetSteps( total, current );
  EXPECT_EQ( total, 50u );
  EXPECT_EQ( current, 20u );
}

TEST( MainDialog, SetupResultMessagesCloseDialogAndQuit )
{
  ActiveWindowGuard guard;
  UINT messages[] = { SETUP_COMPLETE, SETUP_ERROR, SETUP_CANCELLED };
  for ( UINT message : messages )
  {
    TestableMainDialog dialog;
    ASSERT_TRUE( dialog.Create());
    HWND handle = dialog.GetDialogHandle();

    SendMessage( handle, message, 0, 0 );
    EXPECT_TRUE( dialog.GetDialogHandle() == NULL );
    EXPECT_FALSE( IsWindow( handle ));
    EXPECT_TRUE( TakeQuitMessage());
  }
}
