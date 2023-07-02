md build

gcc -std=c99 -g ^
main.c ^
test_template.c ^
..\..\..\common\elab_export.c ^
..\..\..\3rd\Unity\unity.c ^
..\..\..\3rd\Unity\unity_memory.c ^
..\..\..\3rd\Unity\unity_fixture.c ^
-I ..\..\..\3rd\Unity ^
-I ..\..\..\common ^
-D WIN32 ^
-o build\ut
