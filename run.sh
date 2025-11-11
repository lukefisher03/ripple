#! /bin/bash

echo "Compiling..."
clang -o main xml_tree.c -lssl -lcrypto || exit 1
echo "Finished compiling!"

echo -e "\nRunning...\n\n\n"
./main