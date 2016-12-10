
@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set path=W:\dev\c++\ld_37_game\cmd_scripts;%path%
doskey vs=devenv $1
doskey gs=git status
doskey gb=git branch