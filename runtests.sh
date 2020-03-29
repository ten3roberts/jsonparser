# Compile
premake5 gmake2 --test && make

find ./bin -type f -name "test_*" -exec echo "Running test {}" \; -exec time -f "\nResults: %E real,\t%U user,\t%S sys" {} \;