## Web installer sample in pure WINAPI

1) DPI support (HI DPI monitors)
2) VECTOR logo support with via custom font 
3) small and fast
4) supported Windows XP - Windows 10

Main Window of installer:
![screenshot](https://github.com/crea7or/webinstaller/raw/master/installer-main.png)

Install mode after Install button clicked
![screenshot](https://github.com/crea7or/webinstaller/raw/master/installer-download.png)

### Tests

`tests/webinstaller_tests.vcxproj` (part of the solution) is a console application with tests for the logic of
utilities, installer manager, controls and dialogs. Painting is not tested. Run it after the build:

```
x86\Release\webinstaller_tests\webinstaller_tests.exe [filter]
```

`filter` is an optional substring of the test name, e.g. `CompositeLinkControl`. Exit code is 0 when all tests pass.

Mouse logo is from Twitter [twemoji](https://github.com/twitter/twemoji/tree/gh-pages/2) project.

A detailed description of this project in Russian language on [habrahabr.ru](https://habrahabr.ru/post/346274/).