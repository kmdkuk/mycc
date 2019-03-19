#! /bin/bash

try() {
  expected="$1"
  input="$2"

  ./mycc "$input" > tmp.s
  gcc -o tmp tmp.s tmp-plus.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

echo 'int plus(int x, int y) { return x + y; }' | gcc -xc -c -o tmp-plus.o -

try 0 "return 0;"
try 42 "return 42;"
try 21 'return 5+20-4;'
try 41 "return 12 + 34 - 5 ;"
try 47 "return 5+6*7;"
try 15 "return 5*(9-6);"
try 4 "return (3+5)/2;"
try 3 "
a = 3;
return a;
"
try 14 "
a = 3;
b = 5 * 6 - 8;
return a + b /2;
"
try 3 "
aaa = 3;
return aaa;
"
try 14 "
abc= 3;
xyz = 5 * 6 - 8;
return abc + xyz /2;
"
try 14 "
aBc= 3;
Xyz = 5 * 6 - 8;
return aBc + Xyz /2;
"
try 1 "return 1==1;"
try 0 "return 2==1;"
try 1 "return 3 != 4;"
try 0 "return 10!=10;"
try 11 "
a = 3;
b = 4;
return plus(a+b, b);
"
try 3 "
a = 1;
a;
return 3;
b = 2;
b;
"

echo OK
