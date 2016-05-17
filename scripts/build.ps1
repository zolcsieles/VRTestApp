$buildCmd = "C:\Program Files (x86)\MSBuild\12.0\bin\msbuild.exe"
$buildArgs = @(
	"VRTestApp.sln"
	"/logger:C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
	);

if ($env:APPVEYOR_REPO_COMMIT_MESSAGE -and ($env:APPVEYOR_REPO_COMMIT_MESSAGE).Contains("[cov]"))
{
	& cov-build --dir cov-int $buildCmd $buildArgs
	& 7z a cov-vrtestapp.zip cov-int
}
else
{
	& $buildCmd $buildArgs
}
