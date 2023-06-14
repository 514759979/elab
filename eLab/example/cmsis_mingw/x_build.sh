mkdir build

gcc -std=gnu99 -g \
main.c \
test_event.c \
test_malloc.c \
../../common/elab_common.c \
../../port/posix/elab_port.c \
../../RTOS/CMSIS_OS_POSIX/cmsis_os.c \
-I ../.. \
-I ../../RTOS/CMSIS_OS_POSIX \
-I ../../common \
-I ../../elib \
-I ../../port/posix \
-I ../../midware/esh \
-I export \
-I . \
-o build/cmsis \
-l pthread
