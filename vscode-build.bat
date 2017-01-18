@echo off

pushd "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"
call vcvarsall.bat amd64
popd

call build.bat