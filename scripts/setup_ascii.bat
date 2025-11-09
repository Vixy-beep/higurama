@echo off
echo ========================================
echo   ASCII ART SETUP - GENERADOR RAPIDO
echo ========================================
echo.
echo Este script te ayudara a agregar tus ASCII arts personalizados
echo.
echo [1] Agrega tus archivos .txt a la carpeta: ascii_arts/
echo [2] Nombra los archivos segun la accion:
echo     - exploit.txt    (al explotar)
echo     - recon.txt      (al escanear)
echo     - success.txt    (exito)
echo     - shodan.txt     (busqueda shodan)
echo     - vixy.txt       (tu waifu)
echo     - etc.
echo.
echo [3] Reinicia el C2 para cargar los nuevos archivos
echo.
echo ========================================
echo   GENERADORES RECOMENDADOS:
echo ========================================
echo.
echo Para IMAGENES (fotos anime, waifus):
echo   https://www.ascii-art-generator.org/
echo   Ancho: 80-100 caracteres
echo   Caracteres: #%%*+=-.
echo.
echo Para TEXTO grande:
echo   https://patorjk.com/software/taag/
echo   Fuente: "ANSI Shadow" o "Big"
echo.
echo ========================================
echo   ARCHIVOS ACTUALES:
echo ========================================
echo.
dir /b ascii_arts\*.txt
echo.
echo Total: 
dir /b ascii_arts\*.txt | find /c ".txt"
echo archivos cargados
echo.
pause
