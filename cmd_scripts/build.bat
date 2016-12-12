
@echo off
SetLocal EnableDelayedExpansion
cls

set sdl_include=W:\dev\libs\SDL2_VS\include
set src_dir=W:\dev\c++\ld_37_game\src

set libs=SDL2.lib SDL2main.lib SDL2_image.lib SDL2_mixer.lib
set source_files=%src_dir%\ld_37.cpp
set macros=
set compiler_flags=/Zi /MD /EHsc /O2
set linker_flags=/SUBSYSTEM:WINDOWS
set executable=ld_37.exe

pushd bin

cl %compiler_flags% /Fe%executable% /I%sdl_include% %macros% %source_files% %libs% /link %linker_flags%

popd