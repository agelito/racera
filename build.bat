@echo off

cd %~dp0

if not exist bin (
   mkdir bin
)

if not exist obj (
   mkdir obj
)

set SRC_FILES=src/win32_main.c src/racera_game.c src/math.c src/opengl.c src/renderer.c src/shader.c src/mesh.c src/texture.c src/keyboard.c
set LIBRARIES=kernel32.lib gdi32.lib user32.lib opengl32.lib
set INCLUDE_DIR=lib/include

echo "Compiling..."
cl -EHa- /Od /Zi /W3 /D_CRT_SECURE_NO_WARNINGS /Foobj/ /Febin/racera.exe /I %INCLUDE_DIR% %SRC_FILES% %LIBRARIES% /link /subsystem:console

echo "Copying Game Assets..."
xcopy assets\* bin\ /v /d /y /s /e /f