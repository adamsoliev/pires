#!/bin/bash

cat <<EOF | riscv64-unknown-linux-gnu-gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x + y; }
int sub(int x, int y) { return x - y; }
int add6(int a, int b, int c, int d, int e, int f) {
    return a + b + c + d + e + f;
}
EOF

assert() {
    expected="$1"
    input="$2"

    ./pires "$input" > tmp.s || exit
    # riscv64-unknown-linux-gnu-gcc -static -o tmp tmp.s tmp2.o
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

# assert 0 "int main() { return 0; }"
# assert 42 "int main() { return 42; }"
# assert 114 "int main() { return 81+42-9; }"
# assert 41 "int main() { return 12 + 34 - 5; }"
# assert 7 "int main() { return 1+2*3; }"
# assert 9 "int main() { return 1+2*3+2; }"
# assert 13 "int main() { return 1+2*3*2; }"
# assert 3 "int main() { return 1+4/2; }"
# assert 2 "int main() { return 1+4/2/2; }"
# assert 4 "int main() { return 1+10/2-2; }"
# assert 9 "int main() { return (1+2)*3; }"
# assert 70 "int main() { return 1 + 2 * 3 - 4 / 2 + 5 * (6 + 7); }"


# assert 0 "int main() { int a = 0; return a; }"
# assert 42 "int main() { int b = 42; return b; }"
# assert 114 "int main() { int a = 81; int b = 42; int c = 9; return a + b - c; }"
# assert 41 "int main() { int a = 12 + 34 - 5; return a; }"
# assert 7 "int main() { int a = 1+2*3; return a; }"
# assert 9 "int main() { int a = 1+2*3+2; return a; }"
# assert 13 "int main() { int a = 2*3*2; int b = 1; return b + a;}"
# assert 3 "int main() { int a = 1; int b = 4/2; return a + b; }"
# assert 2 "int main() { int a = 1+4/2/2; return a; }"
# assert 4 "int main() { int a = 1+10/2-2; return a; }"
# assert 9 "int main() { int a = (1+2); int b = 3; return a * b; }"
# assert 70 "int main() { int a = 1 + 2 * 3 - 4 / 2 + 5 * (6 + 7); return a; }"

# assert 0 "int main() { return 0==1; }"
# assert 1 "int main() { return 42==42; }"
# assert 1 "int main() { return 0!=1; }"
# assert 0 "int main() { return 42!=42; }"

# assert 1 "int main() { return 0<1; }"
# assert 0 "int main() { return 1<1; }"
# assert 0 "int main() { return 2<1; }"
# assert 1 "int main() { return 0<=1; }"
# assert 1 "int main() { return 1<=1; }"
# assert 0 "int main() { return 2<=1; }"

# assert 1 "int main() { return 1>0; }"
# assert 0 "int main() { return 1>1; }"
# assert 0 "int main() { return 1>2; }"
# assert 1 "int main() { return 1>=0; }"
# assert 1 "int main() { return 1>=1; }"
# assert 0 "int main() { return 1>=2; }"
# assert 0 "int main() { int a = 23; int b = 32; return a == b; }"
# assert 1 "int main() { int a = 23; int b = 32; return a != b; }"
# assert 0 "int main() { int a = 23; int b = 32; return a > b; }"
# assert 0 "int main() { int a = 23; int b = 32; return a >= b; }"
# assert 1 "int main() { int a = 23; int b = 32; return a < b; }"
# assert 1 "int main() { int a = 23; int b = 32; return a <= b; }"

# assert 1 "int main() { int a = -23; int b = 24; return a + b; }"

assert 3 "int main() { int x=3; int *ptr = &x; return *ptr; }"
assert 3 "int main() { int x=3; return *&x; }"
assert 3 "int main() { int x=3; int *y=&x; int **z=&y; return **z; }"
assert 5 "int main() { int x=3; int y=5; return *(&x+1); }"
assert 3 "int main() { int x=3; int y=5; return *(&y-1); }"
assert 5 "int main() { int x=3; int y=5; return *(&x-(-1)); }"
assert 5 "int main() { int x=3; int *y=&x; *y=5; return x; }"
assert 7 "int main() { int x=3; int y=5; *(&x+1)=7; return y; }"
assert 7 "int main() { int x=3; int y=5; *(&y-2+1)=7; return x; }"
assert 5 "int main() { int x=3; return (&x+2)-&x+3; }"

# assert 3 "int main() { if (0) return 2; return 3; }"
# assert 3 "int main() { if (1-1) return 2; return 3; }"
# assert 2 "int main() { if (1) return 2; return 3; }"
# assert 2 "int main() { if (2-1) return 2; return 3; }"

# assert 55 "int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }"
# assert 3 "int main() { for (;;) return 3; return 5; }"

# assert 10 "int main() { int i=0; while(i<10) i=i+1; return i; }"

# assert 3 "int main() { {1; {2;} return 3;} }"
# assert 5 "int main() { ;;; return 5; }"

# assert 10 "int main() { int i=0; while(i<10) i=i+1; return i; }"
# assert 55 "int main() { int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} return j; }"


# assert 3 "int main() { return ret3(); }"
# assert 5 "int main() { return ret5(); }"
# assert 8 "int main() { return add(3, 5); }"
# assert 2 "int main() { return sub(5, 3); }"
# assert 21 "int main() { return add6(1,2,3,4,5,6); }"
# assert 66 "int main() { return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }"
# assert 136 "int main() { return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }"

# assert 32 "int main() { return ret32(); } int ret32() { return 32; }"

# assert 7 "int main() { return add2(3,4); } int add2(int x, int y) { return x+y; }"
# assert 1 "int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }"
# assert 55 "int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }"

echo OK