@echo off
cls
echo.
echo ========================================
echo    LUCKY STAR BOTNET - EJECUCION FINAL
echo ========================================
echo.
echo [*] Los 3 contenedores Docker estan listos!
echo     - 127.0.0.1:2201 (admin:admin)
echo     - 127.0.0.1:2202 (test:test)
echo     - 127.0.0.1:2203 (root:root / user:password)
echo.
echo [*] Iniciando sistema completo...
echo.
timeout /t 2 /nobreak >nul

echo [1/2] Iniciando C2 Commander...
start "Lucky Star C2" cmd /k "color 0A && python c2_commander.py"
timeout /t 3 /nobreak >nul

echo [2/2] Iniciando Bot Soldier...
start "Lucky Star Bot" cmd /k "color 0C && python bot_soldier.py"
timeout /t 2 /nobreak >nul

echo.
echo ========================================
echo    SISTEMA INICIADO
echo ========================================
echo.
echo AHORA EN LA VENTANA DEL C2:
echo.
echo   1. Espera a ver: [+] Nuevo bot conectado
echo   2. Ejecuta: recon 1
echo   3. Espera 10-15 segundos
echo   4. Ejecuta: list 1
echo   5. Veras los 3 contenedores Docker!
echo   6. Ejecuta: exploit 1 1
echo   7. Espera y veras: [+] Explotacion exitosa
echo   8. Ejecuta: docker exec -it vulnerable_ssh_1 bash
echo   9. Dentro ejecuta: ps aux ^| grep python
echo  10. Veras el bot replicado corriendo!
echo.
echo ========================================
echo.
pause
