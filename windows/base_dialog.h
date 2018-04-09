// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "windows/abstract_dialog.h"
#include "installer/installer_manager.h"

#include "ui/static_control.h"
#include "ui/rect_button_control.h"
#include "ui/layered_static_control.h"

class BaseDialog : public AbstractDialog
{
public:
  BaseDialog( HINSTANCE _instanceHandle, UINT _templateId, HWND _parentHandle = NULL );
  virtual ~BaseDialog();

  virtual LRESULT OnInitDialog( HWND dialogHandle ) override;
  virtual LRESULT OnMouse( UINT message, WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnEraseBackground( WPARAM wParam, LPARAM lParam ) override;

protected:

  bool AskToCancelInstall();

  // shared controls and fonts
  virtual void LoadFonts() override;

  StaticControl titleControl;
  LayeredStaticControl logoControl;
  RectButtonControl closeLinkControl;

  InstallerManager& installer = InstallerManager::Get();

  // window move support
  bool windowIsMoving;
  bool positionChanged;
  int pointerXPosition;
  int pointerYPosition;
  // window move support

  // erase background brushes/pens
  HBRUSH windowBackgroundBrush;
  HPEN grayPen;

  // non-copyable
private:

  BaseDialog( const BaseDialog& ) = delete;
  BaseDialog& operator=( const BaseDialog& ) = delete;
};
