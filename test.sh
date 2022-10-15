#!/bin/bash
assert() {
    input="$2"
    expect="$1"

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
assert 21 '5+20-4'
