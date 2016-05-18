# Test vars
#$env:PLATFORM="x64"
#$env:CONFIGURATION="Debug"

# 32-bit (default)
$exedir = $env:CONFIGURATION
$bitness = "32"

# 64-bit
if ($env:PLATFORM -eq "Win64" -or $env:PLATFORM -eq "x64")
{
	$exedir = $env:PLATFORM+"\"+$exedir
	$bitness = "64"
}

# Deploy
$deploy = "vrtestapp-binary-$env:PLATFORM-$env:APPVEYOR_BUILD_VERSION"


# Copy content
md $deploy
cp .\Working\SDL2_$bitness.dll $deploy\SDL2.dll
cp .\Working\openvr_api_$bitness.dll $deploy\openvr_api.dll
cp .\Working\d3dcompiler_47_$bitness.dll $deploy\d3dcompiler_47.dll
cp $exedir\VRTestApp.exe $deploy\
cp .\Working\Data $deploy\ -R

# Compress
7z a $deploy".zip" $deploy
