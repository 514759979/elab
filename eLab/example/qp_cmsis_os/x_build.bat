md build

gcc -std=c99 -g ^
main.c ^
test_qp.c ^
test_thread.c ^
..\..\common\elab_common.c ^
..\..\common\elab_export.c ^
..\..\port\FreeRTOS\elab_port.c ^
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
..\..\3rd\QPC\src\qf\qep_hsm.c ^
..\..\3rd\QPC\src\qf\qep_msm.c ^
..\..\3rd\QPC\src\qf\qf_act.c ^
..\..\3rd\QPC\src\qf\qf_actq.c ^
..\..\3rd\QPC\src\qf\qf_defer.c ^
..\..\3rd\QPC\src\qf\qf_dyn.c ^
..\..\3rd\QPC\src\qf\qf_ps.c ^
..\..\3rd\QPC\src\qf\qf_qact.c ^
..\..\3rd\QPC\src\qf\qf_qmact.c ^
..\..\3rd\QPC\src\qf\qf_time.c ^
..\..\3rd\QPC\src\qf\qf_qeq.c ^
..\..\3rd\QPC\src\qf\qf_mem.c ^
..\..\3rd\QPC\ports\freertos\qf_port.c ^
..\..\3rd\QPC\qpc_export.c ^
-I ..\.. ^
-I ..\..\port\FreeRTOS ^
-I ..\..\RTOS\CMSIS_OS_FreeRTOS ^
-I ..\..\3rd\FreeRTOS\include ^
-I ..\..\3rd\FreeRTOS\portable\mingw ^
-I ..\..\3rd\QPC\include ^
-I ..\..\3rd\QPC\src ^
-I ..\..\3rd ^
-I ..\..\3rd\QPC\ports\freertos ^
-I . ^
-o build\qpc_cmsis_os ^
-l winmm
