# VST Runner Development Environment Setup Script
# For Windows
# Usage: .\setup.ps1

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  VST Runner Development Setup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Step 1: Check prerequisites
Write-Host "Step 1: Checking prerequisites..." -ForegroundColor Yellow
Write-Host "-----------------------------------"

# Check CMake
try {
    $cmakeVersion = (cmake --version | Select-String -Pattern "version (.+)" | ForEach-Object { $_.Matches.Groups[1].Value })
    Write-Host "✅ CMake $cmakeVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ CMake not found!" -ForegroundColor Red
    Write-Host "Please install CMake 3.15 or higher:"
    Write-Host "  Download from: https://cmake.org/download/"
    Write-Host "  or: winget install Kitware.CMake"
    exit 1
}

# Check Git
try {
    $gitVersion = (git --version | Select-String -Pattern "version (.+)" | ForEach-Object { $_.Matches.Groups[1].Value })
    Write-Host "✅ Git $gitVersion" -ForegroundColor Green
} catch {
    Write-Host "❌ Git not found!" -ForegroundColor Red
    Write-Host "Please install Git:"
    Write-Host "  Download from: https://git-scm.com/download/win"
    Write-Host "  or: winget install Git.Git"
    exit 1
}

# Check Visual Studio
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        $vsVersion = & $vsWhere -latest -property catalog_productDisplayVersion
        Write-Host "✅ Visual Studio $vsVersion" -ForegroundColor Green
    } else {
        Write-Host "⚠️  Visual Studio not found" -ForegroundColor Yellow
        Write-Host "Please install Visual Studio 2019 or later with C++ development tools"
        Write-Host "Download from: https://visualstudio.microsoft.com/"
    }
} else {
    Write-Host "⚠️  Visual Studio not found" -ForegroundColor Yellow
    Write-Host "Please install Visual Studio 2019 or later with C++ development tools"
    Write-Host "Download from: https://visualstudio.microsoft.com/"
}

Write-Host ""

# Step 2: Install optional tools
Write-Host "Step 2: Checking optional tools..." -ForegroundColor Yellow
Write-Host "------------------------------------"

# Check clang-format
try {
    $clangFormatVersion = (clang-format --version | Select-String -Pattern "version (.+)" | ForEach-Object { $_.Matches.Groups[1].Value })
    Write-Host "✅ clang-format $clangFormatVersion" -ForegroundColor Green
} catch {
    Write-Host "⚠️  clang-format not found (optional for code formatting)" -ForegroundColor Yellow
    Write-Host "   Install with: winget install LLVM.LLVM"
}

Write-Host ""

# Step 3: Initialize Git submodules
Write-Host "Step 3: Initializing Git submodules..." -ForegroundColor Yellow
Write-Host "---------------------------------------"

if (Test-Path "JUCE\.git") {
    Write-Host "✅ JUCE submodule already initialized" -ForegroundColor Green
} else {
    Write-Host "Initializing JUCE submodule..."
    git submodule update --init --recursive
    Write-Host "✅ JUCE submodule initialized" -ForegroundColor Green
}

Write-Host ""

# Step 4: Set up pre-commit hooks
Write-Host "Step 4: Setting up pre-commit hooks..." -ForegroundColor Yellow
Write-Host "---------------------------------------"

$hookPath = ".git\hooks\pre-commit"
if (Test-Path $hookPath) {
    Write-Host "⚠️  Pre-commit hook already exists, skipping" -ForegroundColor Yellow
} else {
    Write-Host "Installing pre-commit hook..."
    if (Test-Path "scripts\pre-commit") {
        Copy-Item "scripts\pre-commit" $hookPath
        Write-Host "✅ Pre-commit hook installed" -ForegroundColor Green
    } else {
        Write-Host "⚠️  Pre-commit script not found in scripts/" -ForegroundColor Yellow
    }
}

Write-Host ""

# Step 5: Configure build
Write-Host "Step 5: Configuring build..." -ForegroundColor Yellow
Write-Host "----------------------------"

$buildDir = "build"

if (Test-Path $buildDir) {
    Write-Host "⚠️  Build directory already exists" -ForegroundColor Yellow
    $response = Read-Host "Remove and reconfigure? (y/N)"
    if ($response -eq "y" -or $response -eq "Y") {
        Remove-Item -Recurse -Force $buildDir
        Write-Host "Removed existing build directory"
    } else {
        Write-Host "Keeping existing build directory"
    }
}

if (-not (Test-Path $buildDir)) {
    Write-Host "Creating build directory..."
    New-Item -ItemType Directory -Path $buildDir | Out-Null

    Push-Location $buildDir

    Write-Host "Running CMake..."
    cmake .. `
        -DCMAKE_BUILD_TYPE=Debug `
        -DBUILD_TESTS=ON `
        -DBUILD_BENCHMARKS=ON

    Pop-Location
    Write-Host "✅ Build configured" -ForegroundColor Green
} else {
    Write-Host "✅ Build already configured" -ForegroundColor Green
}

Write-Host ""

# Step 6: Build project
Write-Host "Step 6: Building project..." -ForegroundColor Yellow
Write-Host "---------------------------"

$response = Read-Host "Build now? (Y/n)"
if ($response -ne "n" -and $response -ne "N") {
    Write-Host "Building..."
    $cores = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
    cmake --build $buildDir --config Debug -- /maxcpucount:$cores
    Write-Host "✅ Build complete" -ForegroundColor Green
} else {
    Write-Host "⏭️  Skipping build" -ForegroundColor Yellow
}

Write-Host ""

# Step 7: Run tests
Write-Host "Step 7: Running tests..." -ForegroundColor Yellow
Write-Host "------------------------"

$response = Read-Host "Run tests now? (Y/n)"
if ($response -ne "n" -and $response -ne "N") {
    $testExe = "$buildDir\VSTRunnerTests_artefacts\Debug\VSTRunnerTests.exe"
    if (Test-Path $testExe) {
        Write-Host "Running test suite..."
        & $testExe
    } else {
        Write-Host "⚠️  Tests not built yet. Build the project first." -ForegroundColor Yellow
    }
} else {
    Write-Host "⏭️  Skipping tests" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Setup Complete!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host ""
Write-Host "  Build the project:"
Write-Host "    cmake --build build --config Debug"
Write-Host ""
Write-Host "  Run tests:"
Write-Host "    .\build\VSTRunnerTests_artefacts\Debug\VSTRunnerTests.exe"
Write-Host ""
Write-Host "  Run benchmarks:"
Write-Host "    .\build\VSTRunnerBenchmarks_artefacts\Release\VSTRunnerBenchmarks.exe"
Write-Host ""
Write-Host "  Format code (if clang-format installed):"
Write-Host "    Get-ChildItem -Recurse Source\*.cpp,*.h | ForEach-Object { clang-format -i `$_.FullName }"
Write-Host ""
Write-Host "  Open in Visual Studio:"
Write-Host "    start build\VSTRunner.sln"
Write-Host ""
Write-Host "Happy coding! 🎵" -ForegroundColor Green
Write-Host ""
