// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

// control behaviour tests: subclassing, state changes, notifications and resource ownership
// painting is not tested - parent windows are never shown and WM_PAINT is never dispatched

#include "stdafx.h"
#include "tests/test_framework.h"
#include "tests/test_window.h"

#include "ui/base_control.h"
#include "ui/static_control.h"
#include "ui/progress_control.h"
#include "ui/link_control.h"
#include "ui/rect_button_control.h"
#include "ui/composite_link_control.h"
#include "ui/composite_checkbox_control.h"
#include "ui/option_checkbox_control.h"
#include "installer/installer_manager.h"

namespace
{
  const UINT controlId = 100;

  // exposes protected members of StaticControl
  class TestableStatic : public StaticControl
  {
  public:
    using StaticControl::GetControlStyles;
    using StaticControl::GetControlTextColor;
    using StaticControl::SetState;
    using StaticControl::currentState;
    HFONT Font() const { return textFont; }
  };

  class TestableProgress : public ProgressControl
  {
  public:
    LONG MarqueeStep() const { return step; }
  };

  class TestableLink : public LinkControl
  {
  public:
    STATE State() const { return currentState; }
  };

  class TestableButton : public RectButtonControl
  {
  public:
    STATE State() const { return currentState; }
  };

  // piece sizes are normally measured in OnPaint, tests set them directly
  template < typename Base >
  class TestableComposite : public Base
  {
  public:
    using Base::GetPieceByPosition;
    using Base::pieces;
    using Base::activePiece;
    using Base::drawRectangleOffset;

    void SetPieceWidths( const std::vector< LONG >& widths )
    {
      for ( size_t index = 0; index < widths.size() && index < pieces.size(); ++index )
      {
        pieces[ index ].size.cx = widths[ index ];
      }
    }

    StaticControl::STATE State() const { return this->currentState; }
    StaticControl::STATE PieceState( size_t index ) const { return pieces[ index ].state; }
  };

  class TestableCheckbox : public TestableComposite< CompositeCheckboxControl >
  {
  public:
    void SetCheckboxWidth( LONG width ) { checkboxSize = width; }
  };

  int GdiObjectCount()
  {
    return ( int ) GetGuiResources( GetCurrentProcess(), GR_GDIOBJECTS );
  }

  // current ShowCursor display counter
  int CursorDisplayCount()
  {
    int count = ShowCursor( FALSE );
    ShowCursor( TRUE );
    return count + 1;
  }
}

// BaseControl subclassing

TEST( BaseControl, AttachSubclassesWindow )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"text" );
  TestableStatic control;

  EXPECT_FALSE( control.IsAttached());
  EXPECT_TRUE( control.Attach( parent.handle, controlId ));
  EXPECT_TRUE( control.IsAttached());
  EXPECT_TRUE( control.GetWindow() == window );
  EXPECT_TRUE(( WNDPROC ) GetWindowLongPtr( window, GWLP_WNDPROC ) == BaseControl::ControlProc );
  EXPECT_TRUE(( BaseControl* ) GetWindowLongPtr( window, GWLP_USERDATA ) == &control );
}

TEST( BaseControl, AttachFailsForMissingWindowOrWhenAttached )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"text" );
  TestableStatic control;

  EXPECT_FALSE( control.Attach( NULL ));
  EXPECT_FALSE( control.Attach( parent.handle, 9999 ));
  EXPECT_TRUE( control.Attach( window ));
  EXPECT_FALSE( control.Attach( window ));
}

TEST( BaseControl, DetachRestoresWindowProcedure )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"text" );
  LONG_PTR originalProc = GetWindowLongPtr( window, GWLP_WNDPROC );
  TestableStatic control;
  control.Attach( window );

  EXPECT_TRUE( control.Detach());
  EXPECT_FALSE( control.IsAttached());
  EXPECT_TRUE( control.GetWindow() == NULL );
  EXPECT_EQ( GetWindowLongPtr( window, GWLP_WNDPROC ), originalProc );
  EXPECT_EQ( GetWindowLongPtr( window, GWLP_USERDATA ), ( LONG_PTR ) 0 );
  EXPECT_FALSE( control.Detach());
}

TEST( BaseControl, DestructorDetaches )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"text" );
  LONG_PTR originalProc = GetWindowLongPtr( window, GWLP_WNDPROC );
  {
    TestableStatic control;
    control.Attach( window );
  }
  EXPECT_EQ( GetWindowLongPtr( window, GWLP_WNDPROC ), originalProc );
  EXPECT_EQ( GetWindowLongPtr( window, GWLP_USERDATA ), ( LONG_PTR ) 0 );
}

TEST( BaseControl, WindowDestructionDetachesControl )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"text" );
  TestableStatic control;
  control.Attach( window );

  EXPECT_TRUE( DestroyWindow( window ) != FALSE );
  EXPECT_FALSE( control.IsAttached());
  EXPECT_TRUE( control.GetWindow() == NULL );
  EXPECT_FALSE( IsWindow( window ));
}

TEST( BaseControl, GetTextReturnsExactWindowText )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"Hello" );
  BaseControl control;
  control.Attach( window );

  std::wstring text = control.GetText();
  EXPECT_EQ( text.size(), 5u );
  EXPECT_EQ( text, std::wstring( L"Hello" ));

  SetWindowText( window, L"" );
  EXPECT_TRUE( control.GetText().empty());
}

TEST( BaseControl, SetFontFromWindowMessageDoesNotTakeOwnership )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"text" );
  LOGFONT logFont;
  WindowsUtils::FillLogFont( logFont, 10 );
  HFONT callerFont = CreateFontIndirect( &logFont );
  {
    TestableStatic control;
    control.Attach( window );
    SendMessage( window, WM_SETFONT, ( WPARAM ) callerFont, FALSE );
    EXPECT_TRUE( control.Font() == callerFont );

    // replacing the font must not delete the caller's font
    control.SetFont( &logFont );
    EXPECT_TRUE( control.Font() != callerFont );
    EXPECT_EQ( GetObjectType( callerFont ), ( DWORD ) OBJ_FONT );

    SendMessage( window, WM_SETFONT, ( WPARAM ) callerFont, FALSE );
  }
  // nor destroying the control
  EXPECT_EQ( GetObjectType( callerFont ), ( DWORD ) OBJ_FONT );
  DeleteObject( callerFont );
}

TEST( BaseControl, OwnedFontsAreReleased )
{
  int before = GdiObjectCount();
  {
    TestableStatic control;
    LOGFONT logFont;
    WindowsUtils::FillLogFont( logFont, 10 );
    for ( int index = 0; index < 20; ++index )
    {
      control.SetFont( &logFont );
    }
  }
  EXPECT_EQ( GdiObjectCount(), before );
}

TEST( BaseControl, SetBackgroundColorDoesNotLeakBrushes )
{
  TestableStatic control;
  // GDI keeps a few deleted solid brushes in a per process cache that still counts as objects,
  // so fill the cache first and allow small noise - a leak would add one object per call
  for ( int index = 0; index < 50; ++index )
  {
    control.SetBackgroundColor( RGB( index, 1, 0 ));
  }
  int before = GdiObjectCount();
  for ( int index = 0; index < 50; ++index )
  {
    control.SetBackgroundColor( RGB( index, 0, 0 ));
  }
  EXPECT_TRUE( GdiObjectCount() - before < 5 );
  EXPECT_EQ( control.GetBackgroundColor(), ( COLORREF ) RGB( 49, 0, 0 ));
}

TEST( BaseControl, ColorsRoundTrip )
{
  TestableStatic control;
  control.SetForegroundColor( RGB( 1, 2, 3 ));
  control.SetBackgroundColor( RGB( 4, 5, 6 ));
  EXPECT_EQ( control.GetForegroundColor(), ( COLORREF ) RGB( 1, 2, 3 ));
  EXPECT_EQ( control.GetBackgroundColor(), ( COLORREF ) RGB( 4, 5, 6 ));
}

TEST( BaseControl, ShowTogglesVisibleStyle )
{
  TestParentWindow parent;
  HWND window = parent.CreateStatic( controlId, L"text" );
  TestableStatic control;
  control.Attach( window );

  control.Show( false );
  EXPECT_EQ( GetWindowLong( window, GWL_STYLE ) & WS_VISIBLE, 0 );
  control.Show( true );
  EXPECT_EQ( GetWindowLong( window, GWL_STYLE ) & WS_VISIBLE, ( LONG ) WS_VISIBLE );
}

// StaticControl

TEST( StaticControl, AttachCapturesWindowText )
{
  TestParentWindow parent;
  TestableStatic control;
  control.Attach( parent.CreateStatic( controlId, L"Initial" ));
  EXPECT_EQ( control.GetText(), std::wstring( L"Initial" ));
}

TEST( StaticControl, SetTextReplacesText )
{
  TestParentWindow parent;
  TestableStatic control;
  control.Attach( parent.CreateStatic( controlId, L"Initial" ));
  control.SetText( L"Changed" );
  EXPECT_EQ( control.GetText(), std::wstring( L"Changed" ));
}

TEST( StaticControl, AlignmentStylesMapToDrawTextFlags )
{
  TestParentWindow parent;
  struct Case { DWORD style; UINT expected; };
  Case cases[] = {
    { SS_LEFT, DT_LEFT },
    { SS_CENTER, DT_CENTER },
    { SS_RIGHT, DT_RIGHT },
    { SS_CENTER | SS_NOTIFY | SS_CENTERIMAGE, DT_CENTER },
    { SS_RIGHT | SS_NOTIFY, DT_RIGHT },
    // static types that share bits with SS_CENTER/SS_RIGHT are not alignment
    { SS_ICON, DT_LEFT },
    { SS_GRAYRECT, DT_LEFT },
    { SS_LEFTNOWORDWRAP, DT_LEFT },
  };
  for ( const Case& test : cases )
  {
    TestableStatic control;
    control.Attach( parent.CreateStatic( controlId, L"", test.style ));
    DWORD style = control.GetControlStyles( DT_SINGLELINE );
    EXPECT_EQ( style & ( DT_LEFT | DT_CENTER | DT_RIGHT ), ( DWORD ) test.expected );
    EXPECT_EQ( style & DT_SINGLELINE, ( DWORD ) DT_SINGLELINE );
    DestroyWindow( control.GetWindow());
  }
}

TEST( StaticControl, TextColorFollowsState )
{
  TestParentWindow parent;
  TestableStatic control;
  control.Attach( parent.CreateStatic( controlId, L"text" ));
  control.SetForegroundColor( RGB( 1, 1, 1 ));
  control.SetHoverColor( RGB( 2, 2, 2 ));
  control.SetClickColor( RGB( 3, 3, 3 ));

  EXPECT_EQ( control.GetControlTextColor(), ( COLORREF ) RGB( 1, 1, 1 ));
  control.SetState( StaticControl::STATE::HOVER );
  EXPECT_EQ( control.GetControlTextColor(), ( COLORREF ) RGB( 2, 2, 2 ));
  control.SetState( StaticControl::STATE::CLICK );
  EXPECT_EQ( control.GetControlTextColor(), ( COLORREF ) RGB( 3, 3, 3 ));
}

TEST( StaticControl, DisabledTextColorIsSystemGrayText )
{
  TestParentWindow parent;
  TestableStatic control;
  control.Attach( parent.CreateStatic( controlId, L"text" ));
  control.SetState( StaticControl::STATE::HOVER );
  EnableWindow( control.GetWindow(), FALSE );
  // disabled wins over any state
  EXPECT_EQ( control.GetControlTextColor(), GetSysColor( COLOR_GRAYTEXT ));
}

// ProgressControl

TEST( ProgressControl, StepStoresPositionsAndAdvancesMarquee )
{
  TestParentWindow parent;
  TestableProgress control;
  control.Attach( parent.CreateStatic( controlId, L"" ));

  DWORD total = 1, current = 1;
  control.GetSteps( total, current );
  EXPECT_EQ( total, 0u );
  EXPECT_EQ( current, 0u );

  control.Step( 50, 10 );
  control.GetSteps( total, current );
  EXPECT_EQ( total, 50u );
  EXPECT_EQ( current, 10u );
  EXPECT_EQ( control.MarqueeStep(), 1 );

  // back to marquee mode
  control.Step( 0, 0 );
  control.GetSteps( total, current );
  EXPECT_EQ( total, 0u );
  EXPECT_EQ( current, 0u );
  EXPECT_EQ( control.MarqueeStep(), 2 );
}

// LinkControl

TEST( LinkControl, ClickNotifiesParent )
{
  TestParentWindow parent;
  TestableLink control;
  HWND window = parent.CreateStatic( controlId, L"link" );
  control.Attach( window );

  Click( window, 7, 3 );
  WPARAM wParam = 0;
  LPARAM lParam = 0;
  ASSERT_TRUE( parent.TakeCommand( wParam, lParam ));
  EXPECT_EQ( LOWORD( wParam ), controlId );
  EXPECT_EQ( lParam, MAKELPARAM( 7, 3 ));
  EXPECT_FALSE( parent.HasCommand());
  EXPECT_EQ( control.State(), StaticControl::STATE::NORMAL );
}

TEST( LinkControl, ButtonUpWithoutDownDoesNotNotify )
{
  TestParentWindow parent;
  TestableLink control;
  HWND window = parent.CreateStatic( controlId, L"link" );
  control.Attach( window );

  SendMouse( window, WM_LBUTTONUP, 5 );
  EXPECT_FALSE( parent.HasCommand());
}

TEST( LinkControl, ButtonDownSetsClickState )
{
  TestParentWindow parent;
  TestableLink control;
  HWND window = parent.CreateStatic( controlId, L"link" );
  control.Attach( window );

  SendMouse( window, WM_LBUTTONDOWN, 5 );
  EXPECT_EQ( control.State(), StaticControl::STATE::CLICK );
  // double click acts like button down
  SendMouse( window, WM_LBUTTONUP, 5 );
  SendMouse( window, WM_LBUTTONDBLCLK, 5 );
  EXPECT_EQ( control.State(), StaticControl::STATE::CLICK );
}

TEST( LinkControl, HoverAndLeave )
{
  TestParentWindow parent;
  TestableLink control;
  HWND window = parent.CreateStatic( controlId, L"link" );
  control.Attach( window );

  SendMouse( window, WM_MOUSEMOVE, 5 );
  EXPECT_EQ( control.State(), StaticControl::STATE::HOVER );
  SendMessage( window, WM_MOUSELEAVE, 0, 0 );
  EXPECT_EQ( control.State(), StaticControl::STATE::NORMAL );
}

TEST( LinkControl, HoverCyclesKeepCursorDisplayCounter )
{
  TestParentWindow parent;
  TestableLink control;
  HWND window = parent.CreateStatic( controlId, L"link" );
  control.Attach( window );

  int before = CursorDisplayCount();
  for ( int index = 0; index < 5; ++index )
  {
    SendMouse( window, WM_MOUSEMOVE, 5 );
    SendMessage( window, WM_MOUSELEAVE, 0, 0 );
  }
  EXPECT_EQ( CursorDisplayCount(), before );
}

// RectButtonControl

TEST( RectButtonControl, ClickNotifiesParent )
{
  TestParentWindow parent;
  TestableButton control;
  HWND window = parent.CreateStatic( controlId, L"Install", SS_CENTER | SS_NOTIFY );
  control.Attach( window );

  Click( window, 10 );
  WPARAM wParam = 0;
  LPARAM lParam = 0;
  ASSERT_TRUE( parent.TakeCommand( wParam, lParam ));
  EXPECT_EQ( LOWORD( wParam ), controlId );
  EXPECT_EQ( control.State(), StaticControl::STATE::NORMAL );
}

TEST( RectButtonControl, ReleasingAfterLeaveDoesNotNotify )
{
  TestParentWindow parent;
  TestableButton control;
  HWND window = parent.CreateStatic( controlId, L"Install", SS_CENTER | SS_NOTIFY );
  control.Attach( window );

  SendMouse( window, WM_LBUTTONDOWN, 10 );
  // leaving resets the click
  SendMessage( window, WM_MOUSELEAVE, 0, 0 );
  SendMouse( window, WM_LBUTTONUP, 10 );
  EXPECT_FALSE( parent.HasCommand());
}

// CompositeLinkControl

namespace
{
  const UINT firstLinkMessage = 5001;
  const UINT secondLinkMessage = 5002;

  // "text " | "link1" | "link2" | " tail" with widths 10, 20, 30, 15
  // pixel ranges: [0,10) [10,30) [30,60) [60,75)
  void SetupComposite( TestParentWindow& parent, TestableComposite< CompositeLinkControl >& control )
  {
    control.Attach( parent.CreateStatic( controlId, L"" ));
    control.AddPiece( L"text " );
    control.AddPiece( L"link1", firstLinkMessage );
    control.AddPiece( L"link2", secondLinkMessage );
    control.AddPiece( L" tail" );
    control.SetPieceWidths( { 10, 20, 30, 15 } );
  }
}

TEST( CompositeLinkControl, PieceByPositionUsesHalfOpenRanges )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );

  EXPECT_EQ( control.GetPieceByPosition( -1 ), -1 );
  EXPECT_EQ( control.GetPieceByPosition( 0 ), 0 );
  EXPECT_EQ( control.GetPieceByPosition( 9 ), 0 );
  // boundaries belong to the next piece, no dead pixels
  EXPECT_EQ( control.GetPieceByPosition( 10 ), 1 );
  EXPECT_EQ( control.GetPieceByPosition( 29 ), 1 );
  EXPECT_EQ( control.GetPieceByPosition( 30 ), 2 );
  EXPECT_EQ( control.GetPieceByPosition( 74 ), 3 );
  EXPECT_EQ( control.GetPieceByPosition( 75 ), -1 );
}

TEST( CompositeLinkControl, PieceByPositionHonoursAlignmentOffset )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );
  control.drawRectangleOffset = 100;

  EXPECT_EQ( control.GetPieceByPosition( 99 ), -1 );
  EXPECT_EQ( control.GetPieceByPosition( 100 ), 0 );
  EXPECT_EQ( control.GetPieceByPosition( 135 ), 2 );
}

TEST( CompositeLinkControl, AttachWithTextAddsWindowTextPiece )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  EXPECT_TRUE( control.AttachWithText( parent.CreateStatic( controlId, L"Window text" )));
  ASSERT_EQ( control.pieces.size(), 1u );
  EXPECT_EQ( control.pieces[ 0 ].text, std::wstring( L"Window text" ));
  EXPECT_EQ( control.pieces[ 0 ].message, ( UINT ) MAXDWORD );
}

TEST( CompositeLinkControl, HoverHighlightsOnlyLinkPieces )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );
  HWND window = control.GetWindow();

  SendMouse( window, WM_MOUSEMOVE, 15 );
  EXPECT_EQ( control.activePiece, 1 );
  EXPECT_EQ( control.PieceState( 1 ), StaticControl::STATE::HOVER );

  // plain text piece clears highlight
  SendMouse( window, WM_MOUSEMOVE, 5 );
  EXPECT_EQ( control.PieceState( 1 ), StaticControl::STATE::NORMAL );
  EXPECT_EQ( control.PieceState( 0 ), StaticControl::STATE::NORMAL );
  EXPECT_EQ( control.State(), StaticControl::STATE::NORMAL );
}

TEST( CompositeLinkControl, HoverMovesBetweenAdjacentLinks )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );
  HWND window = control.GetWindow();

  SendMouse( window, WM_MOUSEMOVE, 15 );
  SendMouse( window, WM_MOUSEMOVE, 35 );
  EXPECT_EQ( control.activePiece, 2 );
  EXPECT_EQ( control.PieceState( 1 ), StaticControl::STATE::NORMAL );
  EXPECT_EQ( control.PieceState( 2 ), StaticControl::STATE::HOVER );
}

TEST( CompositeLinkControl, MouseLeaveResetsAllPieces )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );
  HWND window = control.GetWindow();

  SendMouse( window, WM_MOUSEMOVE, 35 );
  SendMessage( window, WM_MOUSELEAVE, 0, 0 );
  EXPECT_EQ( control.activePiece, -1 );
  for ( size_t index = 0; index < control.pieces.size(); ++index )
  {
    EXPECT_EQ( control.PieceState( index ), StaticControl::STATE::NORMAL );
  }
}

TEST( CompositeLinkControl, ClickOnLinkSendsPieceMessage )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );

  Click( control.GetWindow(), 40 );
  WPARAM wParam = 0;
  LPARAM lParam = 0;
  ASSERT_TRUE( parent.TakeCommand( wParam, lParam ));
  EXPECT_EQ( LOWORD( wParam ), controlId );
  EXPECT_EQ( lParam, ( LPARAM ) secondLinkMessage );
  EXPECT_EQ( control.PieceState( 2 ), StaticControl::STATE::NORMAL );

  Click( control.GetWindow(), 10 );
  ASSERT_TRUE( parent.TakeCommand( wParam, lParam ));
  EXPECT_EQ( lParam, ( LPARAM ) firstLinkMessage );
}

TEST( CompositeLinkControl, ClickOnTextDoesNotNotify )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );

  Click( control.GetWindow(), 3 );
  Click( control.GetWindow(), 70 );
  Click( control.GetWindow(), 200 );
  EXPECT_FALSE( parent.HasCommand());
}

TEST( CompositeLinkControl, PressAndReleaseOnDifferentLinksDoesNotNotify )
{
  TestParentWindow parent;
  TestableComposite< CompositeLinkControl > control;
  SetupComposite( parent, control );

  SendMouse( control.GetWindow(), WM_LBUTTONDOWN, 15 );
  SendMouse( control.GetWindow(), WM_LBUTTONUP, 35 );
  EXPECT_FALSE( parent.HasCommand());
}

// CompositeCheckboxControl

namespace
{
  const UINT checkboxLinkMessage = 6001;

  // [checkbox 16] "Option" | " ( " | "license" | " )" with widths 40, 10, 30, 10
  // pixel ranges: checkbox [0,16) text [16,56) [56,66) link [66,96) [96,106)
  void SetupCheckbox( TestParentWindow& parent, TestableCheckbox& control )
  {
    control.AttachWithText( parent.CreateStatic( controlId, L"Option" ));
    control.AddPiece( L" ( " );
    control.AddPiece( L"license", checkboxLinkMessage );
    control.AddPiece( L" )" );
    control.SetCheckboxWidth( 16 );
    control.SetPieceWidths( { 40, 10, 30, 10 } );
  }
}

TEST( CompositeCheckboxControl, PiecePositionsStartAfterCheckbox )
{
  TestParentWindow parent;
  TestableCheckbox control;
  SetupCheckbox( parent, control );

  EXPECT_EQ( control.GetPieceByPosition( 5 ), -1 );
  EXPECT_EQ( control.GetPieceByPosition( 16 ), 0 );
  EXPECT_EQ( control.GetPieceByPosition( 70 ), 2 );
}

TEST( CompositeCheckboxControl, SetCheckAndGetCheck )
{
  TestParentWindow parent;
  TestableCheckbox control;
  SetupCheckbox( parent, control );

  EXPECT_FALSE( control.GetCheck());
  control.SetCheck( true );
  EXPECT_TRUE( control.GetCheck());
  control.SetCheck( false );
  EXPECT_FALSE( control.GetCheck());
}

TEST( CompositeCheckboxControl, ClickOnBoxOrTextToggles )
{
  TestParentWindow parent;
  TestableCheckbox control;
  SetupCheckbox( parent, control );

  Click( control.GetWindow(), 5 );
  EXPECT_TRUE( control.GetCheck());
  Click( control.GetWindow(), 20 );
  EXPECT_FALSE( control.GetCheck());
  Click( control.GetWindow(), 100 );
  EXPECT_TRUE( control.GetCheck());
  EXPECT_FALSE( parent.HasCommand());
}

TEST( CompositeCheckboxControl, ClickOnLinkNotifiesWithoutToggling )
{
  TestParentWindow parent;
  TestableCheckbox control;
  SetupCheckbox( parent, control );

  Click( control.GetWindow(), 70 );
  EXPECT_FALSE( control.GetCheck());
  WPARAM wParam = 0;
  LPARAM lParam = 0;
  ASSERT_TRUE( parent.TakeCommand( wParam, lParam ));
  EXPECT_EQ( LOWORD( wParam ), controlId );
  EXPECT_EQ( lParam, ( LPARAM ) checkboxLinkMessage );
}

TEST( CompositeCheckboxControl, SetFontDoesNotLeak )
{
  int before = GdiObjectCount();
  {
    TestableCheckbox control;
    LOGFONT logFont;
    WindowsUtils::FillLogFont( logFont, 10 );
    for ( int index = 0; index < 10; ++index )
    {
      control.SetFont( &logFont );
    }
    // checkbox font is derived from the given one, caller's struct is restored
    LONG height = logFont.lfHeight;
    control.SetFont( &logFont );
    EXPECT_EQ( logFont.lfHeight, height );
    EXPECT_EQ( logFont.lfUnderline, FALSE );
  }
  EXPECT_EQ( GdiObjectCount(), before );
}

// OptionCheckboxControl

namespace
{
  const UINT optionId = 4002;
}

TEST( OptionCheckboxControl, CheckStateIsStoredInInstallerManager )
{
  InstallerManager& manager = InstallerManager::Get();
  manager.SetOptionSelected( optionId, false );
  TestParentWindow parent;
  OptionCheckboxControl control( optionId );
  control.AttachWithText( parent.CreateStatic( optionId, L"Third funny option" ));

  control.SetCheck( true );
  EXPECT_TRUE( manager.GetOptionSelected( optionId ));

  manager.SetOptionSelected( optionId, false );
  EXPECT_FALSE( control.GetCheck());
  manager.SetOptionSelected( optionId, true );
  EXPECT_TRUE( control.GetCheck());
  manager.SetOptionSelected( optionId, false );
}

TEST( OptionCheckboxControl, ClickTogglesInstallerOption )
{
  InstallerManager& manager = InstallerManager::Get();
  manager.SetOptionSelected( optionId, false );
  TestParentWindow parent;
  OptionCheckboxControl control( optionId );
  control.AttachWithText( parent.CreateStatic( optionId, L"Third funny option" ));

  Click( control.GetWindow(), 5 );
  EXPECT_TRUE( manager.GetOptionSelected( optionId ));
  Click( control.GetWindow(), 5 );
  EXPECT_FALSE( manager.GetOptionSelected( optionId ));
}
