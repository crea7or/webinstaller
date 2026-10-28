// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#pragma once

// minimal dependency free test framework
// TEST registers a test, EXPECT_* records a failure and continues, ASSERT_* stops the test

#include <sstream>
#include <string>
#include <vector>

namespace TestFramework
{
  typedef void ( *TestFunction )();

  struct TestCase
  {
    const char* suite;
    const char* name;
    TestFunction function;
  };

  // thrown by ASSERT_* to stop the current test
  struct AssertionFailed
  {};

  std::vector< TestCase >& Registry();

  // records failure of the current test
  void ReportFailure( const char* file, int line, const std::string& message );

  struct Registrar
  {
    Registrar( const char* suite, const char* name, TestFunction function )
    {
      Registry().push_back( { suite, name, function } );
    }
  };

  template < typename T >
  std::string ToString( const T& value )
  {
    std::ostringstream stream;
    stream << value;
    return stream.str();
  }

  inline std::string ToString( const std::wstring& value )
  {
    // messages are for the console, non-ASCII characters are replaced
    std::string result;
    for ( wchar_t symbol : value )
    {
      result.push_back( symbol < 0x80 ? static_cast< char >( symbol ) : '?' );
    }
    return result;
  }

  inline std::string ToString( const wchar_t* value )
  {
    return value == nullptr ? std::string( "(null)" ) : ToString( std::wstring( value ));
  }

  inline std::string ToString( std::nullptr_t )
  {
    return "nullptr";
  }

  template < typename A, typename B >
  bool CheckEqual( const A& actual, const B& expected, const char* actualText, const char* expectedText, const char* file, int line )
  {
    if ( actual == expected )
    {
      return true;
    }
    ReportFailure( file, line, std::string( actualText ) + " == " + expectedText + "\n      actual:   " + ToString( actual ) + "\n      expected: " + ToString( expected ));
    return false;
  }

  template < typename A, typename B >
  bool CheckNotEqual( const A& actual, const B& unexpected, const char* actualText, const char* unexpectedText, const char* file, int line )
  {
    if ( !( actual == unexpected ))
    {
      return true;
    }
    ReportFailure( file, line, std::string( actualText ) + " != " + unexpectedText + "\n      both: " + ToString( actual ));
    return false;
  }

  inline bool CheckTrue( bool condition, const char* conditionText, const char* file, int line )
  {
    if ( !condition )
    {
      ReportFailure( file, line, std::string( "expected true: " ) + conditionText );
    }
    return condition;
  }
}

#define TEST( suite, name ) \
  static void suite##_##name(); \
  static TestFramework::Registrar suite##_##name##_registrar( #suite, #name, suite##_##name ); \
  static void suite##_##name()

#define EXPECT_TRUE( condition ) TestFramework::CheckTrue( !!( condition ), #condition, __FILE__, __LINE__ )
#define EXPECT_FALSE( condition ) TestFramework::CheckTrue( !( condition ), "!(" #condition ")", __FILE__, __LINE__ )
#define EXPECT_EQ( actual, expected ) TestFramework::CheckEqual(( actual ), ( expected ), #actual, #expected, __FILE__, __LINE__ )
#define EXPECT_NE( actual, unexpected ) TestFramework::CheckNotEqual(( actual ), ( unexpected ), #actual, #unexpected, __FILE__, __LINE__ )

#define ASSERT_TRUE( condition ) do { if ( !EXPECT_TRUE( condition )) throw TestFramework::AssertionFailed(); } while ( 0 )
#define ASSERT_EQ( actual, expected ) do { if ( !EXPECT_EQ( actual, expected )) throw TestFramework::AssertionFailed(); } while ( 0 )
