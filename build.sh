#!/bin/bash

os_name=$(uname -s)

case "$os_name" in
  Linux)
    gcc lib/C_vector/C_vector.c main.c -lraylib -lm -std=c17 -o main
    ;;
  Darwin)
    gcc lib/C_vector/C_vector.c main.c -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib -lm -framework Cocoa -framework OpenGL -framework IOKit -std=c17 -o main
    ;;
  *)
    echo "Platform: Unknown ($os_name)"
    ;;
esac

./main
