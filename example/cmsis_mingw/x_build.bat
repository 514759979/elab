md build

gcc -std=c99 -g ^
main.c ^
test_event.c ^
test_malloc.c ^
..\..\RTOS\CMSIS_OS_FreeRTOS\cmsis_os_cb.c ^
..\..\RTOS\CMSIS_OS_FreeRTOS\cmsis_os.c ^
..\..\3rd\FreeRTOS\croutine.c ^
..\..\3rd\FreeRTOS\event_groups.c ^
..\..\3rd\FreeRTOS\list.c ^
..\..\3rd\FreeRTOS\queue.c ^
..\..\3rd\FreeRTOS\stream_buffer.c ^
..\..\3rd\FreeRTOS\tasks.c ^
..\..\3rd\FreeRTOS\timers.c ^
..\..\3rd\FreeRTOS\portable\MemMang\heap_4.c ^
..\..\3rd\FreeRTOS\portable\mingw\port.c ^
-I ..\..\RTOS\CMSIS_OS_FreeRTOS ^
-I ..\..\3rd\FreeRTOS\include ^
-I ..\..\3rd\FreeRTOS\portable\mingw ^
-I . ^
-o build\cmsis_os ^
-l winmm