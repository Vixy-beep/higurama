@echo off
echo ========================================
echo    LIMPIANDO ARCHIVOS INNECESARIOS
echo ========================================
echo.

echo [*] Creando carpeta de archivos antiguos...
mkdir archive 2>nul
mkdir docs 2>nul
mkdir tools 2>nul

echo [*] Moviendo archivos de testing antiguos...
move check_why_not_working.py archive\ 2>nul
move test_exploit.py archive\ 2>nul
move test_replication.py archive\ 2>nul
move test_all_containers.py archive\ 2>nul
move test_ssh_manual.py archive\ 2>nul
move verify_docker.py archive\ 2>nul
move verify_replication.py archive\ 2>nul
move fix_and_compile.bat archive\ 2>nul
move fix_docker.bat archive\ 2>nul
move diagnose.bat archive\ 2>nul
move verify_config.bat archive\ 2>nul

echo [*] Moviendo docs antiguos...
move AUTO_REPLICATION.md docs\ 2>nul
move COMPILE_GUIDE.md docs\ 2>nul
move QUICK_DEMO.md docs\ 2>nul
move README_ES.md docs\ 2>nul
move README_FINAL.md docs\ 2>nul
move REPLICATION_GUIDE.md docs\ 2>nul
move TESTING_GUIDE.md docs\ 2>nul
move SOLUCION_ERROR.md docs\ 2>nul
move UNIVERSAL_GUIDE.md docs\ 2>nul

echo [*] Moviendo scripts de compilacion antiguos...
move build_universal_bot.py tools\ 2>nul
move compile_bot.py tools\ 2>nul
move compile_linux_bot.bat tools\ 2>nul
move COMPILE_WITH_CONSOLE.bat tools\ 2>nul
move quick_test.bat tools\ 2>nul
move test_binary.bat tools\ 2>nul
move test_replication.bat tools\ 2>nul

echo [*] Moviendo scripts de setup antiguos...
move install_dependencies.py tools\ 2>nul
move install_full.bat tools\ 2>nul
move quick_start.bat tools\ 2>nul
move create_honeypot.py tools\ 2>nul
move setup_vulnerable_vm.sh tools\ 2>nul
move setup_c2.sh tools\ 2>nul
move universal_payload.sh tools\ 2>nul
move payload.sh tools\ 2>nul
move add_defender_exclusion.bat tools\ 2>nul

echo [*] Limpiando builds antiguos...
rmdir /s /q build 2>nul
rmdir /s /q dist 2>nul
del systemd.spec 2>nul

echo.
echo ========================================
echo    LIMPIEZA COMPLETA
echo ========================================
echo.
echo Archivos ESENCIALES (mantener):
echo - bot_soldier.py
echo - c2_commander.py
echo - shodan_hunter.py
echo - COMPILE_NOW.bat
echo - COMPILE_UNIVERSAL.bat
echo - start_binary_server.bat
echo - start_docker_lab.bat
echo - start_demo.bat
echo - FINAL_RUN.bat
echo - install_shodan.bat
echo - setup_tor_c2.bat
echo - install_bot_tor.sh
echo - systemd.exe (binario Windows)
echo - systemd (binario Linux)
echo - cert.pem / key.pem
echo - docker-vulnerable-ssh.yml
echo - requirements.txt
echo - README.md
echo - SHODAN_GUIDE.md
echo - TOR_SETUP_GUIDE.md
echo.
echo Archivos movidos a carpetas:
echo - archive/ (tests viejos)
echo - docs/ (documentacion antigua)
echo - tools/ (scripts de utilidad)
echo.
pause
