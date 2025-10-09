param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

# Enable parallel test execution to speed up CI (836 tests)
# Temp file creation is disabled at compile time to prevent disk space issues
$env:CTEST_PARALLEL_LEVEL = 2

./cxx/run-unit-tests.ps1 `
    -RepoName $RepoName `
    -ProjectDir $ProjectDir `
    -Name $Name `
    -Configuration $Configuration `
    -Arch $Arch `
    -BuildMethod $BuildMethod `
    -ExcludeRegex ".*Example.*" `
    -CoverageExcludeDirs 'fiftyone-common-c(xx)?-cov\.dir$'

exit $LASTEXITCODE
