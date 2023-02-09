md build

gcc -std=c99 -g ^
main.c ^
elab_common_win32.c ^
..\..\common\elab_export.c ^
-I ..\.. ^
-I ..\..\common ^
-I . ^
-o build\elab_export_test