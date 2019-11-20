#! /bin/sh

try() {
  expected="$1"
  input="$2"

  ./bin/mycc "$input" > tmp.s
  gcc -o tmp tmp.s tmp-plus.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "exec: $input"
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

echo 'int plus(int x, int y) { return x + y; }' | gcc -xc -c -o tmp-plus.o -
testdir="test"
try 0 "$testdir/simple/return0.kmcc"
try 42 "$testdir/simple/return42.kmcc"
try 21 "$testdir/simple/return21.kmcc"
try 41 "$testdir/simple/return41.kmcc"
try 47 "$testdir/simple/return47.kmcc"
try 15 "$testdir/simple/return15.kmcc"
try 4 "$testdir/simple/return4.kmcc"
try 3 "$testdir/function/simple_function.kmcc"
try 3 "$testdir/vars/single_letral.kmcc"
try 14 "$testdir/vars/single_letral2.kmcc"
try 3 "$testdir/vars/multi_letral.kmcc"
try 14 "$testdir/vars/multi_letral2.kmcc"
try 14 "$testdir/vars/multi_letral3.kmcc"
try 1 "$testdir/compare/equa.kmcc"
try 0 "$testdir/compare/equa2.kmcc"
try 1 "$testdir/compare/nequal.kmcc"
try 0 "$testdir/compare/nequal2.kmcc"
try 11 "$testdir/function/call.kmcc"
try 3 "$testdir/function/definition.kmcc"
try 1 "$testdir/control/if.kmcc"

echo OK
