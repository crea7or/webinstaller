// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"

#include "windows/main_dialog.h"
#include "windows/windows_utils.h"
#include "installer/installer_constants.h"
#include "installer/resource.h"

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

MainDialog::MainDialog( HINSTANCE _instanceHandle, HWND _parentHandle ) :
  BaseDialog( _instanceHandle, IDD_DIALOG_MAIN, _parentHandle )
{
  progressControl.SetBackgroundColor( Installer::progressBackgroundColor );
  progressControl.SetForegroundColor( Installer::progressForegroundColor );

  // icon
  mainIcon = LoadIcon( _instanceHandle, MAKEINTRESOURCE( IDI_WEBINSTALLER ));

  timerHandle = NULL;
  hbmBackground = NULL;
  hdcMemDC = NULL;
  hbmOld = NULL;

  // initial mode is window with install button
  mode = false;
}

MainDialog::~MainDialog()
{
  if ( hbmBackground != NULL )
  {
    SelectObject( hdcMemDC, hbmOld );
    DeleteObject( hbmBackground );
  }
  if ( hdcMemDC != NULL )
  {
    DeleteObject( hdcMemDC );
  }
  DestroyIcon( mainIcon );
  RemoveCheckboxes();
}

LRESULT MainDialog::OnInitDialog( HWND initDialogHandle )
{
  BaseDialog::OnInitDialog( initDialogHandle );

  // window params
  SetWindowText( dialogHandle, Installer::eMsgWindowTitle );
  SetClassLong( dialogHandle, GCL_HICON, ( LPARAM ) mainIcon );

  // app description
  descriptionControl.SetForegroundColor( Installer::mainTextColor );
  descriptionControl.Attach( initDialogHandle, IDC_STATIC_DESCRIPTION );

  // main windows eula and policy
  mainLicense.SetForegroundColor( Installer::mainTextColor );
  mainLicense.AddPiece( L"By clicking Install button you will be accepting our " );
  mainLicense.AddPiece( L"end-user license", EULA_CLICK );
  mainLicense.AddPiece( L" and " );
  mainLicense.AddPiece( L"privacy policy", POLICY_CLICK );
  mainLicense.AddPiece( L"." );
  mainLicense.Attach( initDialogHandle, IDC_STATIC_MAIN_LICENSE );

  // setup buton
  installButtonControl.SetBackgroundColor( RGB( 0xff, 0xff, 0xff ));
  installButtonControl.SetForegroundColor( RGB( 0xff, 0xff, 0xff ));
  installButtonControl.SetHoverColor( RGB( 0xdd, 0xe9, 0xff ));
  installButtonControl.SetClickColor( RGB( 0x7d, 0xab, 0xff ));
  installButtonControl.SetTextShadowColor( RGB( 0x00, 0x31, 0x8b ));
  installButtonControl.SetButtonNormalColor( RGB( 0x00, 0x56, 0xf5 ));
  installButtonControl.SetButtonHoverColor( RGB( 0x30, 0x76, 0xf5 ));
  installButtonControl.SetButtonClickColor( RGB( 0x26, 0x5c, 0xc0 ));
  installButtonControl.Attach( initDialogHandle, IDC_SETUP_BUTTON );

  // installer related controls
  infoTextControl.SetForegroundColor( Installer::mainTextColor );
  infoTextControl.Attach( initDialogHandle, IDC_STATIC_PROGRESS_TEXT );
  progressControl.Attach( initDialogHandle, IDC_PROGRESS );

  // add checkboxes dynamically
  CreateCheckboxes();

  installer.SetActiveWindow( dialogHandle );

  // current mode is awaiting user input
  SetMode( false );

  // record clean background in controls
  RedrawDialog( true );

  return 0;
}

void MainDialog::CreateCheckboxes()
{
  const std::vector< InstallOption >& options = installer.GetOptions();

  // getting default position for checkboxes
  HWND staticHandle = GetDlgItem( dialogHandle, IDC_STATIC_CHECKBOX_PLACEHOLDER );
  WindowsUtils::RectHolder staticRect;
  GetWindowRect( staticHandle, &staticRect );
  MapWindowPoints( HWND_DESKTOP, dialogHandle, (LPPOINT) &staticRect, 2 );
  ShowWindow( staticHandle, SW_HIDE );
  LONG yPositionOffset = 0;

  // font
  LOGFONT fontStruct;
  WindowsUtils::FillLogFont( fontStruct, 10, dialogHandle );
  fontStruct.lfWeight = FW_NORMAL;

  LONG offset = staticRect.Height() + ( staticRect.Height() >> 4 );

  for ( const InstallOption& option : options )
  {
    staticHandle = CreateWindowEx( WS_EX_TRANSPARENT, L"STATIC", option.name.c_str(), WS_VISIBLE | WS_CHILD | SS_LEFT | SS_NOTIFY,
      staticRect.left, staticRect.top + yPositionOffset, staticRect.Width(), staticRect.Height(),
      dialogHandle, (HMENU)option.controlId, instanceHandle, NULL );

    OptionCheckboxControl* checkboxPtr = new OptionCheckboxControl( option.controlId );
    checkboxPtr->SetForegroundColor( Installer::mainTextColor );
    checkboxPtr->SetFont( &fontStruct );
    checkboxPtr->AttachWithText( staticHandle );
    if ( option.linkText.size() > 0 )
    {
      checkboxPtr->AddPiece( L" ( " );
      checkboxPtr->AddPiece( option.linkText.c_str(), option.controlId );
      checkboxPtr->AddPiece( L" )" );
    }
    checkboxPtr->SetCheck( checkboxPtr->GetCheck());
    checkboxes.push_back( checkboxPtr );

    // where to put the next control
    yPositionOffset += offset;
  }
}

void MainDialog::RemoveCheckboxes()
{
  while ( checkboxes.size() > 0 )
  {
    if ( checkboxes.back()->GetWindow() != NULL )
    {
      DestroyWindow( checkboxes.back()->GetWindow());
    }
    checkboxes.back()->Detach();
    delete checkboxes.back();
    checkboxes.pop_back();
  }
}

LRESULT MainDialog::OnClose()
{
  if ( AskToCancelInstall())
  {
    // avoid accidential timer processing
    mode = false;
    DestroyDialog( true );
  }
  return 0;
};

LRESULT MainDialog::OnUserMessage( UINT message, WPARAM wParam, LPARAM lParam )
{
  switch ( message )
  {
    // maintain sorting of the messages because this is important
    case SETUP_CANCELLED:
    {
      // report about error
      // ..
    }
    case SETUP_ERROR:
    {
      // report about error in installer already sent from the thread
    }
    case SETUP_COMPLETE:
    {
      // no report needed
      // avoid accidential timer processing
      mode = false;
      DestroyDialog( true );
      break;
    }
    case SET_PROGRESS_TEXT:
    {
      infoTextControl.SetText(( wchar_t* ) lParam );
      break;
    }
    case SET_PROGRESS_BAR:
    {
      progressControl.Step( wParam, lParam );
      break;
    }
  }
  return 0;
}

LRESULT MainDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
  const uint32_t controlId = LOWORD( wParam );
  switch ( controlId )
  {
    case IDC_STATIC_CLOSE:
    {
      OnClose();
      break;
    }
    case IDC_SETUP_BUTTON:
    {
      // go to donwload/install mode
      SetMode( true );
      installer.InstallerStart();
      break;
    }
    case IDC_STATIC_MAIN_LICENSE:
    {
      switch ( lParam )
      {
        case EULA_CLICK:
          ShellExecute( NULL, L"open", Installer::eApiInstallerEula, NULL, NULL, SW_SHOWNORMAL );
          break;
        case POLICY_CLICK:
          ShellExecute( NULL, L"open", Installer::eApiInstallerPolicy, NULL, NULL, SW_SHOWNORMAL );
          break;
      }
      break;
    }
    default:
    {
      std::wstring url = installer.GetOptionLinkUrl( controlId );
      if ( url.size() > 0 )
      {
        ShellExecute( NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL );
      }
      break;
    }
  }
  return 0;
}

// false - initial mode, true - download/install mode
void MainDialog::SetMode( bool _mode )
{
  mode = _mode;
  if ( mode )
  {
    installButtonControl.Show( false );
    // blur window background
    if ( hdcMemDC == NULL && hbmBackground == NULL )
    {
      // temporary hide it
      closeLinkControl.Show( false );
      HDC hdcWindow = GetDC( dialogHandle );
      hdcMemDC = CreateCompatibleDC( hdcWindow );
      if ( hdcMemDC != NULL )
      {
        WindowsUtils::RectHolder clientRect;
        GetClientRect( dialogHandle, &clientRect );
        hbmBackground = CreateCompatibleBitmap( hdcWindow, clientRect.Width(), clientRect.Height());
        if ( hbmBackground != NULL )
        {
          hbmOld = ( HBITMAP ) SelectObject( hdcMemDC, hbmBackground );
          if ( BitBlt( hdcMemDC, 0, 0, clientRect.right, clientRect.bottom, hdcWindow, 0, 0, SRCCOPY ))
          {
            WindowsUtils::BlurBitmap( hdcMemDC, hbmBackground, clientRect.Width(), clientRect.Height(), 12 );
            BitBlt( hdcWindow, 0, 0, clientRect.right, clientRect.bottom, hdcMemDC, 0, 0, SRCCOPY );
            mainLicense.Show( false );
            descriptionControl.Show( false );
            titleControl.Show( false );
            logoControl.Show( false );
            for ( auto checkboxPtr : checkboxes )
            {
              checkboxPtr->Show( false );
            }
          }
        }
        else
        {
          // in case of error with HBITMAP
          DeleteObject( hdcMemDC );
          hdcMemDC = NULL;
        }
      }
      ReleaseDC( dialogHandle, hdcWindow );
      closeLinkControl.Show( true );
    }

    if ( timerHandle == NULL )
    {
      timerHandle = SetTimer( dialogHandle, ( UINT_PTR ) dialogHandle, 1, NULL );
    }
  }

  infoTextControl.Show( mode );
  progressControl.Show( mode );

  RedrawDialog( false );
}

LRESULT MainDialog::OnEraseBackground( WPARAM wParam, LPARAM lParam )
{
  HDC deviceContext = ( HDC ) wParam;
  if ( deviceContext != NULL && hdcMemDC != NULL && hbmBackground != NULL )
  {
    RECT clientRect;
    GetClientRect( dialogHandle, &clientRect );
    BitBlt( deviceContext, 0, 0, clientRect.right, clientRect.bottom, hdcMemDC, 0, 0, SRCCOPY );
  }
  else
  {
    BaseDialog::OnEraseBackground( wParam, lParam );
  }
  return ( LRESULT ) TRUE;
}

LRESULT MainDialog::OnTimer( WPARAM, LPARAM )
{
  if ( mode )
  {
    DWORD total, current;
    progressControl.GetSteps( total, current );
    if ( total == 0 && current == 0 )
    {
      progressControl.Step( 0, 0 );
    }
  }
  return 0;
}

void MainDialog::LoadFonts()
{
  // load shared fonts
  BaseDialog::LoadFonts();

  LOGFONT fontStruct;

  WindowsUtils::FillLogFont( fontStruct, 11, dialogHandle );
  descriptionControl.SetFont( &fontStruct );
  infoTextControl.SetFont( &fontStruct );

  WindowsUtils::FillLogFont( fontStruct, 10, dialogHandle );
  mainLicense.SetFont( &fontStruct );

  // dynamic checkboxes
  for ( auto checkboxPtr : checkboxes )
  {
    checkboxPtr->SetFont( &fontStruct );
  }

  WindowsUtils::FillLogFont( fontStruct, 14, dialogHandle, L"arial" );
  fontStruct.lfWeight = FW_BOLD;
  installButtonControl.SetFont( &fontStruct );
}
