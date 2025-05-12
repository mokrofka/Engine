@REM @echo off
 
@REM echo "Compiling shaders..."

@REM @REM  echo "assets/shaders/Builtin.MaterialShader.vert.glsl -> assets/shaders/Builtin.MaterialShader.vert.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.MaterialShader.vert.glsl -o assets/shaders/Builtin.MaterialShader.vert.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
 
@REM @REM  echo "assets/shaders/Builtin.MaterialShader.frag.glsl -> assets/shaders/Builtin.MaterialShader.frag.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=frag assets/shaders/Builtin.MaterialShader.frag.glsl -o assets/shaders/Builtin.MaterialShader.frag.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

@REM @REM  echo "assets/shaders/Builtin.UIShader.vert.glsl -> assets/shaders/Builtin.UIShader.vert.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert assets/shaders/Builtin.UIShader.vert.glsl -o assets/shaders/Builtin.UIShader.vert.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
 
@REM @REM  echo "assets/shaders/Builtin.UIShader.frag.glsl -> assets/shaders/Builtin.UIShader.frag.spv"
@REM %VULKAN_SDK%\bin\glslc.exe -fshader-stage=frag assets/shaders/Builtin.UIShader.frag.glsl -o assets/shaders/Builtin.UIShader.frag.spv
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
 
@REM echo "Done."

@echo off
setlocal enabledelayedexpansion

set GLSLC=%VULKAN_SDK%\bin\glslc.exe
set SHADER_DIR=assets\shaders

echo Compiling GLSL shaders in %SHADER_DIR%...

for %%F in (%SHADER_DIR%\*.vert) do (
    echo Compiling %%F...
    %GLSLC% -fshader-stage=vert "%%F" -o "%%~dpnF.vert.spv"
    IF %ERRORLEVEL% NEQ 0 (
        echo Error compiling %%F
        exit /b %ERRORLEVEL%
    )
)

for %%F in (%SHADER_DIR%\*.frag) do (
    echo Compiling %%F...
    %GLSLC% -fshader-stage=frag "%%F" -o "%%~dpnF.frag.spv"
    IF %ERRORLEVEL% NEQ 0 (
        echo Error compiling %%F
        exit /b %ERRORLEVEL%
    )
)

echo All shaders compiled successfully.
