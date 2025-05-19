@echo off
setlocal enabledelayedexpansion

set GLSLC=%VULKAN_SDK%\bin\glslc.exe
set SHADER_DIR=assets\shaders
set COMPILED_DIR=%SHADER_DIR%\compiled

echo Compiling GLSL shaders in %SHADER_DIR%...

for %%F in (%SHADER_DIR%\*.vert) do (
  echo Compiling %%F...
  %GLSLC% -fshader-stage=vert "%%F" -o "%COMPILED_DIR%\%%~nF.vert.spv"
  IF %ERRORLEVEL% NEQ 0 (
    echo Error compiling %%F
    exit /b %ERRORLEVEL%
  )
)

for %%F in (%SHADER_DIR%\*.frag) do (
  echo Compiling %%F...
  %GLSLC% -fshader-stage=frag "%%F" -o "%COMPILED_DIR%\%%~nF.frag.spv"
  IF %ERRORLEVEL% NEQ 0 (
    echo Error compiling %%F
    exit /b %ERRORLEVEL%
  )
)

for %%F in (%SHADER_DIR%\*.comp) do (
  echo Compiling %%F...
  %GLSLC% -fshader-stage=comp "%%F" -o "%COMPILED_DIR%\%%~nF.comp.spv"
  IF %ERRORLEVEL% NEQ 0 (
    echo Error compiling %%F
    exit /b %ERRORLEVEL%
  )
)

echo All shaders compiled successfully.
