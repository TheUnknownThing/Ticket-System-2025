#!/bin/bash
cp ../../build/code .

if [ $# -eq 0 ]; then
    echo "Usage: $0 <test_number1> [test_number2] ..."
    exit 1
fi

for num in "$@"; do
    infile="${num}.in"
    outfile="${num}.out"
    testout="${num}.test_out"
    if [ ! -f "$infile" ] || [ ! -f "$outfile" ]; then
        echo "Test point $num: input or output file not found!"
        continue
    fi
    ./code < "$infile" > "$testout"
    if ! diff -q "$testout" "$outfile" > /dev/null; then
        echo "Test $num failed:"
        diff "$testout" "$outfile"
    else
        echo "Test $num passed."
    fi
    rm -f data_data data_node
done

rm -f code
rm -f *.test_out