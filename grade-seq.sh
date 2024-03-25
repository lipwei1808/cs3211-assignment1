#!/bin/bash

for i in ./test_cases/seq/*.in; do
  # Run server as a background process
  ./build/engine 3000 > output.txt &

  # Get server pid
  server_pid=$!

  ./build/mygrader $i
  # awk '{ for(i=1; i<NF; i++) printf "%s ", $i; print "" }' "$i"
  echo =========
  cat output.txt
  echo =============
  rm output.txt
  kill "$server_pid"
done