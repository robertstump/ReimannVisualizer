echo
echo "##########################################################"
echo "#                 Compiling All Tests....                #"
echo "##########################################################"

clang -std=c99 -Wall -Werror tests/test_scratch_arena.c -o test_lib/scratch_arena_tests -Iinclude -Isrc 

if [ $? -eq 0 ]; then
    echo "[X] Tests compilation complete...."
    exit 0

fi

echo "[X] Compilation Failed"

