$x = msbuild VRTestApp.sln /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"

# -or $env:APPVEYOR_REPO_COMMIT_MESSAGE -and ($env:APPVEYOR_REPO_COMMIT_MESSAGE).Contains("[cov]")
if ($True)
{
	$x = cov-build --dir cov-int $x
}

$x
