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

try 0 "main(){return 0;}"
try 42 "main(){return 42;}"
try 21 'main(){return 5+20-4;}'
try 41 "main(){return 12 + 34 - 5 ;}"
try 47 "main(){return 5+6*7;}"
try 15 "main(){return 5*(9-6);}"
try 4 "
main(){
  return (3+5)/2;
}"
try 3 "
foo(){
  return 3;
}
main(){
  return foo();
}
"
try 3 "
main(){
  a = 3;
  return a;
}
"
try 14 "
main(){
  a = 3;
  b = 5 * 6 - 8;
  return a + b /2;
}
"
try 3 "
main(){
  aaa = 3;
  return aaa;
}
"
try 14 "
main(){
  abc= 3;
  xyz = 5 * 6 - 8;
  return abc + xyz /2;
}
"
try 14 "
main(){
  aBc= 3;
  Xyz = 5 * 6 - 8;
  return aBc + Xyz /2;
}
"
try 1 "main(){return 1==1;}"
try 0 "main(){return 2==1;}"
try 1 "main(){return 3 != 4;}"
try 0 "main(){return 10!=10;}"
try 11 "
main(){
  a = 3;
  b = 4;
  return plus(a+b, b);
}
"
try 3 "
bar(a){
  return a;
}
main(){
  return bar(3);
}
"

echo OK
