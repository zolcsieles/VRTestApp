@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
set PATH=%PATH%;C:\!BuildOSVR\cov-analysis-win64-7.7.0.4\bin

echo cov-configure --msvc
echo cov-build --dir cov-int msbuild VRTestApp.sln

msbuild VRTestApp.sln /t:Clean
cmd
