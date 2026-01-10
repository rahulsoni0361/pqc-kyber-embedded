$msys2Shell = "D:\Projects\ccompilers\msys64\msys2_shell.cmd"
$includeDirs = "-Iinclude -Itest/vendor"
$unitySrc = "test/vendor/unity.c"

# Find all test files
$testFiles = Get-ChildItem -Path "test" -Filter "test_*.c"

foreach ($file in $testFiles) {
    $testBase = $file.BaseName
    $srcFile = "src/" + ($testBase -replace "test_", "") + ".c"
    $output = "build/$testBase.exe"
    
    Write-Host "--- Testing $testBase ---" -ForegroundColor Cyan
    
    # Compile the test using MSYS2 shell
    if (Test-Path $srcFile) {
        $compileCmd = "gcc test/$($file.Name) $srcFile $unitySrc $includeDirs -o $output"
    } else {
        # compile without separate src file (self-contained test)
        $compileCmd = "gcc test/$($file.Name) $unitySrc $includeDirs -o $output"
    }
    
    & $msys2Shell -mingw64 -defterm -no-start -here -c $compileCmd

    if ($LASTEXITCODE -eq 0) {
        # Run the test
        & $output
    } else {
        Write-Host "Compilation failed for $testBase" -ForegroundColor Red
    }
}
