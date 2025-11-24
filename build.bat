@echo off
REM Build script for Windows PowerShell / cmd (uses MSYS2 MinGW64 layout)
SET MSYS=%MSYS64% 
IF NOT DEFINED MSYS SET MSYS=C:\msys64
SET MINGW=%MSYS%\mingw64

echo Building apocalip (Windows)...
SET PATH=%MINGW%\bin;%PATH%

REM Try to use pkg-config if present
where pkg-config >nul 2>nul
IF %ERRORLEVEL%==0 (
  for /f "usebackq delims=" %%a in (`pkg-config --cflags sdl2 SDL2_ttf`) do set CFLAGS=%%a
  for /f "usebackq delims=" %%a in (`pkg-config --libs sdl2 SDL2_ttf`) do set LIBS=%%a
  gcc main.c -o apocalip.exe %CFLAGS% %LIBS%
) ELSE (
  gcc main.c -o apocalip.exe -I"%MINGW%\include\SDL2" -L"%MINGW%\lib" -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
)
echo Build finished. Run apocalip.exe