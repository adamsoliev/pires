#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    ./luan "$input" > tmp.s || exit

    riscv64-unknown-linux-gnu-gcc -static -o tmp tmp.s

    qemu-riscv64 -L /home/adam/dev/riscv/sysroot ./tmp

    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 0
assert 32 32
assert 21 "5+20-4"
assert 41 "12 + 34 - 5 "
assert 47 "5+6*7"
assert 15 "5*(9-6)"
assert 70 "1 + 2 * 3 - 4 / 2 + 5 * (6 + 7)"
assert 3 "- -3"
assert 1 "- - +1"
assert 2 "-1++++2-----3+++++4"

echo OK