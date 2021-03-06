# Test
#$env:PLATFORM="Win32"

#Build command and arguments
$buildCmd = "C:\Program Files (x86)\MSBuild\12.0\bin\msbuild.exe"

$buildArgs = @(
	"VRTestApp.sln"
	"/logger:C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
	);

#32-bit (default)
$bitness = 32

# 64-bit
if ($env:PLATFORM -eq "Win64" -or $env:PLATFORM -eq "x64")
{
	$bitness = "64"
}
	
if ($env:APPVEYOR_REPO_COMMIT_MESSAGE -and ($env:APPVEYOR_REPO_COMMIT_MESSAGE).Contains("[cov]") -and ($bitness -eq "32"))
{
	& cov-build --dir cov-int $buildCmd $buildArgs
	$covfile = "cov-vrtestapp-$env:PLATFORM.zip"
	& 7z a $covfile cov-int
	& curl.exe --silent --form token=p0P3q01mzsV165C6db1bbQ --form email=zolcsieles@gmail.com --form file=@$covfile --form version="$env:APPVEYOR_BUILD_VERSION" --form description="$env:PLATFORM-$env:APPVEYOR_REPO_COMMIT_MESSAGE" https://scan.coverity.com/builds?project=zolcsieles%2FVRTestApp
}
else
{
	& $buildCmd $buildArgs
}
