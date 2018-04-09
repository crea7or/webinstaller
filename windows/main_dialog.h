// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "windows/base_dialog.h"

#include "ui/link_control.h"
#include "ui/composite_link_control.h"
#include "ui/composite_checkbox_control.h"
#include "ui/option_checkbox_control.h"
#include "ui/rect_button_control.h"
#include "ui/progress_control.h"

class MainDialog : public BaseDialog
{
public:
  MainDialog( HINSTANCE _instanceHandle, HWND _parentHandle = NULL );
  virtual ~MainDialog();

  virtual LRESULT OnInitDialog( HWND dialogHandle ) override;
  virtual LRESULT OnUserMessage( UINT message, WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnCommand( WPARAM wParam, LPARAM lParam ) override;
  virtual LRESULT OnTimer( WPARAM, LPARAM lParam ) override;
  virtual LRESULT OnClose() override;
  virtual LRESULT OnEraseBackground( WPARAM wParam, LPARAM lParam ) override;

protected:

  void SetMode( bool mode );

  // controls and fonts
  virtual void LoadFonts() override;

  RectButtonControl installButtonControl;
  CompositeLinkControl mainLicense;
  StaticControl descriptionControl;

  // install mode controls
  StaticControl infoTextControl;
  ProgressControl progressControl;

  // timer for progerss detection
  UINT_PTR timerHandle;

  // checkboxes
  void CreateCheckboxes();
  void RemoveCheckboxes();

  // blur background support
  HBITMAP hbmBackground;
  HBITMAP hbmOld;
  HDC hdcMemDC;

  // app icon
  HICON mainIcon;

  // window mode
  bool mode;

  std::vector< OptionCheckboxControl* > checkboxes;
};