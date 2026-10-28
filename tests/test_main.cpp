// Copyright 2018 CEZEO software Ltd. ( http://www.cezeo.com ). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "stdafx.h"
#include "tests/test_framework.h"
#include "installer/installer_manager.h"

#include <cstdio>
#include <cstring>

namespace TestFramework
{
  namespace
  {
    int currentFailures = 0;
  }

  std::vector< TestCase >& Registry()
  {
    static std::vector< TestCase > tests;
    return tests;
  }

  void ReportFailure( const char* file, int line, const std::string& message )
  {
    ++currentFailures;
    printf( "    %s(%d): %s\n", file, line, message.c_str());
  }
}

// usage: webinstaller_tests.exe [filter]
// filter is a substring of "suite.name"
int main( int argc, char** argv )
{
  const char* filter = argc > 1 ? argv[ 1 ] : NULL;

  // tests need the custom font, same as the installer does on start
  InstallerManager::Get().InitializeWithInstance( GetModuleHandle( NULL ));

  int passed = 0, failed = 0;
  for ( const TestFramework::TestCase& test : TestFramework::Registry())
  {
    std::string fullName = std::string( test.suite ) + "." + test.name;
    if ( filter != NULL && strstr( fullName.c_str(), filter ) == NULL )
    {
      continue;
    }

    TestFramework::currentFailures = 0;
    try
    {
      test.function();
    }
    catch ( const TestFramework::AssertionFailed& )
    {
      // already reported
    }
    catch ( ... )
    {
      TestFramework::ReportFailure( __FILE__, __LINE__, "unexpected exception" );
    }

    if ( TestFramework::currentFailures == 0 )
    {
      ++passed;
      printf( "[  OK  ] %s\n", fullName.c_str());
    }
    else
    {
      ++failed;
      printf( "[ FAIL ] %s\n", fullName.c_str());
    }
  }

  printf( "\n%d passed, %d failed\n", passed, failed );
  return failed == 0 ? 0 : 1;
}
