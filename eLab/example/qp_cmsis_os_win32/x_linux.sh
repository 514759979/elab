mkdir build

gcc -std=gnu99 -g \
main.c \
test_qp.c \
test_thread.c \
../../common/elab_common.c \
../../common/elab_assert.c \
../../common/elab_export.c \
../../port/posix/elab_port.c \
../../RTOS/CMSIS_OS_POSIX/cmsis_os.c \
../../3rd/Shell/shell_cmd_list.c \
../../3rd/Shell/shell_companion.c \
../../3rd/Shell/shell_ext.c \
../../3rd/Shell/shell.c \
../../midware/esh/esh.c \
../../3rd/QPC/src/qf/qep_hsm.c \
../../3rd/QPC/src/qf/qep_msm.c \
../../3rd/QPC/src/qf/qf_act.c \
../../3rd/QPC/src/qf/qf_actq.c \
../../3rd/QPC/src/qf/qf_defer.c \
../../3rd/QPC/src/qf/qf_dyn.c \
../../3rd/QPC/src/qf/qf_ps.c \
../../3rd/QPC/src/qf/qf_qact.c \
../../3rd/QPC/src/qf/qf_qmact.c \
../../3rd/QPC/src/qf/qf_time.c \
../../3rd/QPC/src/qf/qf_qeq.c \
../../3rd/QPC/src/qf/qf_mem.c \
../../3rd/QPC/ports/posix/qf_port.c \
../../3rd/QPC/qpc_export.c \
-I ../.. \
-I ../../common \
-I ../../port/posix \
-I ../../RTOS/CMSIS_OS_POSIX \
-I ../../3rd/QPC/include \
-I ../../3rd/QPC/src \
-I ../../3rd \
-I ../../3rd/QPC/ports/posix \
-I ../../midware/esh \
-I . \
-o build/qpc_cmsis_os \
-l pthread
