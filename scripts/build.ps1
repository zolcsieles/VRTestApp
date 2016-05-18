$buildCmd = "C:\Program Files (x86)\MSBuild\12.0\bin\msbuild.exe"

$buildArgs = @(
	"VRTestApp.sln"
	"/logger:C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
	);

if ($env:APPVEYOR_REPO_COMMIT_MESSAGE -and ($env:APPVEYOR_REPO_COMMIT_MESSAGE).Contains("[cov]"))
{
	& cov-build --dir cov-int $buildCmd $buildArgs
	& 7z a cov-vrtestapp.zip cov-int
	& curl.exe --silent --form token=p0P3q01mzsV165C6db1bbQ --form email=zolcsieles@gmail.com --form file=@cov-vrtestapp-$env:PLATFORM.zip --form version="$env:APPVEYOR_BUILD_VERSION" --form description="$env:PLATFORM-$env:APPVEYOR_REPO_COMMIT_MESSAGE" https://scan.coverity.com/builds?project=zolcsieles%2FVRTestApp
}
else
{
	& $buildCmd $buildArgs
}
