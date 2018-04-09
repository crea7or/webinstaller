// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

#include "ui/composite_checkbox_control.h"
#include "installer/installer_manager.h"

class OptionCheckboxControl : public CompositeCheckboxControl
{
public:

  OptionCheckboxControl( const UINT id );
  virtual ~OptionCheckboxControl();

  virtual void SetCheck( const bool state );
  virtual bool GetCheck() const;

protected:

  const UINT id;

  InstallerManager& manager = InstallerManager::Get();

  // non-copyable
private:

  OptionCheckboxControl( const OptionCheckboxControl& ) = delete;
  OptionCheckboxControl& operator=( const OptionCheckboxControl& ) = delete;
};

