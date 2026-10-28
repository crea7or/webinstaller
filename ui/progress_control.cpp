// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "ui/progress_control.h"
#include "windows/windows_utils.h"

ProgressControl::ProgressControl()
{
  totalPosition = 0;
  currentPosition = 0;
  step = 0;
  slices = 2;

  memoryDc = NULL;
  memoryBitmap = NULL;
  previousMemoryBitmap = NULL;
  memoryWidth = 0;
  memoryHeight = 0;
  barBrush = NULL;
  barBrushColor = 0;
}

ProgressControl::~ProgressControl()
{
  DestroyPaintCache();
  if ( barBrush != NULL )
  {
    DeleteObject( barBrush );
  }
}

void ProgressControl::DestroyPaintCache()
{
  if ( memoryDc != NULL )
  {
    SelectObject( memoryDc, previousMemoryBitmap );
    DeleteObject( memoryBitmap );
    DeleteDC( memoryDc );
  }
  memoryDc = NULL;
  memoryBitmap = NULL;
  previousMemoryBitmap = NULL;
  memoryWidth = 0;
  memoryHeight = 0;
}

LRESULT ProgressControl::OnPaint()
{
  PAINTSTRUCT ps;
  BeginPaint( windowHandle, &ps );

  WindowsUtils::RectHolder windowRectangle, barRectangle, backgroundRectangle;
  GetClientRect( windowHandle, &windowRectangle );
  barRectangle = windowRectangle;
  backgroundRectangle = windowRectangle;

  if ( windowRectangle.Width() > 0 )
  {
    // recreate back buffer only when size changes
    if ( memoryDc == NULL || memoryWidth != windowRectangle.Width() || memoryHeight != windowRectangle.Height())
    {
      DestroyPaintCache();
      memoryDc = CreateCompatibleDC( ps.hdc );
      memoryBitmap = CreateCompatibleBitmap( ps.hdc, windowRectangle.Width(), windowRectangle.Height());
      previousMemoryBitmap = ( HBITMAP ) SelectObject( memoryDc, memoryBitmap );
      memoryWidth = windowRectangle.Width();
      memoryHeight = windowRectangle.Height();
    }

    // recreate bar brush only when color changes
    if ( barBrush == NULL || barBrushColor != foregroundColor )
    {
      if ( barBrush != NULL )
      {
        DeleteObject( barBrush );
      }
      barBrush = CreateSolidBrush( foregroundColor );
      barBrushColor = foregroundColor;
    }
    // kept in sync with backgroundColor by BaseControl
    HBRUSH backgroundBrush = windowBackgroundBrush;

    if ( totalPosition > 0 )
    {
      // normal bar
      double relation = ( double )currentPosition / ( double )totalPosition;
      LONG barSize = ( LONG )(( double )windowRectangle.Width() * relation );

      if ( barSize > 0 )
      {
        barRectangle.right = barSize;
        FillRect( memoryDc, &barRectangle, barBrush );
      }
      if ( barSize < windowRectangle.Width())
      {
        backgroundRectangle.left = barSize;
        FillRect( memoryDc, &backgroundRectangle, backgroundBrush );
      }
    }
    else
    {
      // marquee
      LONG pixelsPerBar = windowRectangle.Width() / slices;
      if ( pixelsPerBar > 3 )
      {
        bool paintBar = false;
        LONG firstBarSize = pixelsPerBar;
        if ( step > ( pixelsPerBar << 1 ))
        {
          step = 0;
        }
        if ( step > pixelsPerBar )
        {
          paintBar = true;
          firstBarSize = step - pixelsPerBar;
        }
        else
        {
          firstBarSize = step;
        }
        if ( firstBarSize == 0 )
        {
          firstBarSize = pixelsPerBar;
          paintBar = !paintBar;
        }

        LONG offset = 0;
        for ( DWORD index = 0; index < slices; ++index )
        {
          barRectangle.left = offset;
          barRectangle.right = offset + firstBarSize;
          FillRect( memoryDc, &barRectangle, paintBar ? barBrush : backgroundBrush );
          offset += firstBarSize;
          firstBarSize = pixelsPerBar;
          paintBar = !paintBar;
        }

        if ( barRectangle.right < windowRectangle.right )
        {
          barRectangle.left = offset;
          barRectangle.right = offset + firstBarSize;
          FillRect( memoryDc, &barRectangle, paintBar ? barBrush : backgroundBrush );
          offset += firstBarSize;
          paintBar = !paintBar;
          if ( barRectangle.right < windowRectangle.right )
          {
            barRectangle.left = offset;
            barRectangle.right = windowRectangle.right;
            FillRect( memoryDc, &barRectangle, paintBar ? barBrush : backgroundBrush );
          }
        }
      }
      else
      {
        if ( step & 0x01 )
        {
          FillRect( memoryDc, &windowRectangle, backgroundBrush );
        }
        else
        {
          FillRect( memoryDc, &windowRectangle, barBrush );
        }
      }
    }

    BitBlt( ps.hdc, 0, 0, windowRectangle.Width(), windowRectangle.Height(), memoryDc, 0, 0, SRCCOPY );
  }
  EndPaint( windowHandle, &ps );
  return 0;
}

void ProgressControl::Step( DWORD total, DWORD current )
{
  ++step;
  totalPosition = total;
  currentPosition = current;
  RedrawControl();
}

void ProgressControl::GetSteps( DWORD& total, DWORD& current )
{
  total = totalPosition;
  current = currentPosition;
}

LRESULT ProgressControl::OnEraseBackground( HDC )
{
  return (LRESULT) TRUE;
}
