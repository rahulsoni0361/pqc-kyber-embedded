# Kyber C Implementation Build Script
# 
# This script compiles and runs the Kyber implementation with tests

$ErrorActionPreference = "Stop"

# Configuration
$GCC = "D:\Projects\ccompilers\msys64\mingw64\bin\gcc.exe"
$SRC_DIR = "src"
$INC_DIR = "include"
$TEST_DIR = "test"
$BUILD_DIR = "build"

# Create build directory
if (-not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
}

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "  Kyber C Implementation Build" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# Source files
$SOURCES = @(
    "$SRC_DIR/ntt.c",
    "$SRC_DIR/poly.c",
    "$SRC_DIR/polyvec.c", 
    "$SRC_DIR/fips202.c",
    "$SRC_DIR/indcpa.c",
    "$SRC_DIR/kem.c",
    "$SRC_DIR/utils.c"
)

# Compile flags
$CFLAGS = @(
    "-O3",
    "-Wall",
    "-Wextra",
    "-I$INC_DIR"
)

Write-Host "[1] Compiling Kyber library..." -ForegroundColor Yellow

# Compile each source file to object
$OBJ_FILES = @()
foreach ($src in $SOURCES) {
    $obj = "$BUILD_DIR\" + [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
    $OBJ_FILES += $obj
    
    Write-Host "    Compiling $src..."
    & $GCC -c $src -o $obj $CFLAGS
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Failed to compile $src" -ForegroundColor Red
        exit 1
    }
}

Write-Host "[2] Compiling test program..." -ForegroundColor Yellow

# Compile test
& $GCC -c "$TEST_DIR/test_kem.c" -o "$BUILD_DIR/test_kem.o" $CFLAGS
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to compile test_kem.c" -ForegroundColor Red
    exit 1
}

Write-Host "[3] Linking..." -ForegroundColor Yellow

# Link everything
$ALL_OBJS = $OBJ_FILES + "$BUILD_DIR/test_kem.o"
& $GCC $ALL_OBJS -o "$BUILD_DIR/test_kem.exe"
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to link" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Build successful!" -ForegroundColor Green
Write-Host ""
Write-Host "[4] Running test..." -ForegroundColor Yellow
Write-Host ""

# Run the test
& "$BUILD_DIR/test_kem.exe"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "All tests passed!" -ForegroundColor Green
}
else {
    Write-Host ""
    Write-Host "Test failed!" -ForegroundColor Red
    exit 1
}
