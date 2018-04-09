// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/option_checkbox_control.h"

OptionCheckboxControl::OptionCheckboxControl( const UINT _id ) : id( _id )
{}

OptionCheckboxControl::~OptionCheckboxControl()
{}

void OptionCheckboxControl::SetCheck( const bool state )
{
  manager.SetOptionSelected( id, state );
  CompositeCheckboxControl::SetCheck( state );
}

bool OptionCheckboxControl::GetCheck() const
{
  return manager.GetOptionSelected( id );
}
