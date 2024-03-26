#!/bin/bash

for file in ./test_cases/custom/*.in; do
    if [ -f "$file" ]; then
        ./grader_arm64 ./build/engine < "$file" | tail -n 1
    fi
done
