#!/bin/bash

for i in ./test_cases/seq/*.in; do
  # Run server as a background process
  ./build/engine 3000 > output.txt 2> /dev/null &

  # Get server pid
  server_pid=$!

  # Get basename of file
  file_name=$(basename "$i")
  base_name="${file_name%.*}"

  ./build/mygrader $i > /dev/null
  # awk '{ for(i=1; i<NF; i++) printf "%s ", $i; print "" }' "$i"
  diff <(awk '{ for(i=1; i<NF; i++) printf "%s ", $i; print "" }' output.txt) <(awk '{ for(i=1; i<NF; i++) printf "%s ", $i; print "" }' ./test_cases/seq/$base_name.out)
  if [[ $? != 0 ]]; then
    echo $i failed
  fi
  rm output.txt
  kill "$server_pid"
done