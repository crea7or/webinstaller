// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"

#include "windows/main_dialog.h"
#include "installer/installer_manager.h"

int APIENTRY wWinMain( HINSTANCE hInstance, HINSTANCE, LPWSTR, int )
{
  HANDLE systemWideMutex = CreateMutex( NULL, FALSE, Installer::eMsgWindowTitle );
  if (( systemWideMutex == NULL ) || ( GetLastError() == ERROR_ALREADY_EXISTS ))
  {
    // already launched - exit
    return 0;
  }

  try
  {
    // install manager (install related params and objects)
    InstallerManager& installer = InstallerManager::Get();
    // load font from resource
    installer.InitializeWithInstance( hInstance );

    // Perform application initialization:
    MainDialog mainDialog( hInstance );
    if ( mainDialog.Create() == false )
    {
      // report about error
      //
    }
    else
    {
      // show main window
      mainDialog.ShowDialog();

      // main message loop:
      MSG msg;
      while ( GetMessage( &msg, NULL, 0, 0 ))
      {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
      }
    }
  }
  catch ( ... )
  {
    // crash
    // report about error
    //
  }

  ReleaseMutex( systemWideMutex );
  CloseHandle( systemWideMutex );

  return 0;
}
