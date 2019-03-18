#! /bin/bash

try() {
  expected="$1"
  input="$2"

  ./mycc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 '5+20-4;'
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 3 "
a = 3;
a;
"
try 14 "
a = 3;
b = 5 * 6 - 8;
a + b /2;
"
try 3 "
aaa = 3;
aaa;
"
try 14 "
abc= 3;
xyz = 5 * 6 - 8;
abc + xyz /2;
"
try 14 "
aBc= 3;
Xyz = 5 * 6 - 8;
aBc + Xyz /2;
"

echo OK
