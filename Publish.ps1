Param ([Parameter(Mandatory = $True)] $ApiKey)

$SolutionDirectory="."
$Version="0.0.1"

Get-ChildItem $SolutionDirectory -include bin,obj,x64,packages -Recurse | ForEach-Object ($_) { Remove-Item $_.FullName -Force -Recurse }

nuget restore $SolutionDirectory

msbuild $SolutionDirectory /nr:false /p:Configuration=Release /p:Platform=x64 /t:Demuxer:Rebuild

$Now = Get-Date -Format FileDateTimeUniversal
dotnet pack $SolutionDirectory\Demuxer\Demuxer.csproj -c Release -p:Platform=x64 -p:VersionPrefix=$Version -p:VersionSuffix="local-$Now"

Copy-Item $SolutionDirectory\Demuxer\bin\x64\Release\Demuxer.$Version-local-$Now.nupkg $env:OFFLINE_PACKAGES_DIRECTORY

Write-Output "Successfully copied the NuGet package to $env:OFFLINE_PACKAGES_DIRECTORY"

dotnet nuget push "$SolutionDirectory\Demuxer\bin\x64\Release\Demuxer.$Version-local-$Now.nupkg" --api-key $ApiKey --source "github"
