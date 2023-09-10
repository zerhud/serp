#!/usr/bin/env bash

#g++ -std=c++23 -fdiagnostics-color=always test.cpp -o test 2>&1 |less
#./test 2>&1 |less

g++ -std=gnu++23 -I. -Icomcon -fdiagnostics-color=always comcon/test.cpp -o comcon_test |less
./concon_test

