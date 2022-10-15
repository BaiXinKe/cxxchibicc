#!/bin/bash
assert() {
    input="$1"
    expect="$2"

    ./cxxchibicc $input >tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$expect" = "$actual" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expect expected, but $actual"
        exit 1
    fi
}

assert 0 0
assert 42 42
