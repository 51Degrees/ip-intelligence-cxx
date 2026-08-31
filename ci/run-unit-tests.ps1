param(
    [string]$RepoName,
    [string]$ProjectDir = ".",
    [string]$Name = "Release_x64",
    [string]$Arch = "x64",
    [string]$Configuration = "Release",
    [string]$BuildMethod = "cmake"
)

$ExcludeRegex = ".*Example.*"

if ($IsWindows) {
    # Every ctest entry is a separate process that reads the 2.3GB data file
    # before the test body runs, and Windows pays roughly twice what Linux does
    # for it. The tests below reload or measure the data file on top of that,
    # so they dominate the Windows job (~13 of its ~76 minutes) while adding
    # nothing that the three Ubuntu jobs do not already cover.
    #
    # EngineIpiTestsMemory*.Reload is the exception: it is skipped on Linux
    # (see EngineIpiTestsMemory.cpp), so Windows is its only coverage. Keep the
    # NullNull case and drop the other three.
    $LongRunningOnWindows = @(
        'EngineIpiTestsMemory(InMemoryNull|InMemoryOnePropertyString|NullOnePropertyString)\.Reload'
        'EngineIpiTestsMemory.*\.(MetaDataReload|Size)'
        'EngineIpiTestsFile.*\.(Reload|MetaDataReload|Size)'
    )
    $ExcludeRegex = (@($ExcludeRegex) + $LongRunningOnWindows) -join '|'
}

./cxx/run-unit-tests.ps1 `
    -RepoName $RepoName `
    -ProjectDir $ProjectDir `
    -Name $Name `
    -Configuration $Configuration `
    -Arch $Arch `
    -BuildMethod $BuildMethod `
    -ExcludeRegex $ExcludeRegex `
    -CoverageExcludeDirs 'fiftyone-common-c(xx)?-cov\.dir$'

exit $LASTEXITCODE
