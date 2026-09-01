@echo off
REM Run the test suite headless (no SDL window). KIRO_HEADLESS=1 makes the
REM Engine constructor skip TCODConsole::initRoot; SDL dummy drivers are a
REM belt-and-suspenders fallback for any incidental SDL init.
set KIRO_HEADLESS=1
set SDL_VIDEODRIVER=dummy
set SDL_AUDIODRIVER=dummy
"x64\Debug\40kRL_Tests.exe" %*
echo EXIT CODE: %ERRORLEVEL%
