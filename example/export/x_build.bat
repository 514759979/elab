md build

gcc -std=c99 -g ^
*.c ^
..\..\elab\common\*.c ^
..\..\elab\os\win32\cmsis_os_cb.c ^
..\..\elab\os\win32\cmsis_os.c ^
..\..\elab\3rd\FreeRTOS\*.c ^
..\..\elab\3rd\FreeRTOS\portable\MemMang\heap_4.c ^
..\..\elab\3rd\FreeRTOS\portable\mingw\port.c ^
-I ..\.. ^
-I . ^
-o build\export ^
-l winmm