#!/bin/bash
cmake -S . `-D CMAKE_BUILD_TYPE=Release`
cmake --build .  --config Release --target re2dfa
