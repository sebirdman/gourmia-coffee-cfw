#!/bin/bash

platform=`uname`
echo "Setting up $platform for development"

TOOLCHAIN=$PWD/xtensa-lx106-elf/bin
ESP_IDF=$PWD/idf

if [[ $platform = "Linux" ]]; then

    if [ ! -d "$TOOLCHAIN" ]; then
        wget -q https://dl.espressif.com/dl/xtensa-lx106-elf-linux64-1.22.0-92-g8facf4c-5.2.0.tar.gz
        tar -xzf xtensa-lx106-elf-linux64-1.22.0-92-g8facf4c-5.2.0.tar.gz
        rm xtensa-lx106-elf-linux64-1.22.0-92-g8facf4c-5.2.0.tar.gz
    fi

elif [[ $platform = "Darwin" ]]; then

    if [ ! -d "$TOOLCHAIN" ]; then
        wget -q https://dl.espressif.com/dl/xtensa-lx106-elf-osx-1.22.0-92-g8facf4c-5.2.0.tar.gz
        tar -xzf xtensa-lx106-elf-osx-1.22.0-92-g8facf4c-5.2.0.tar.gz
        rm xtensa-lx106-elf-osx-1.22.0-92-g8facf4c-5.2.0.tar.gz
    fi

fi

if [[ $PATH != *$TOOLCHAIN* ]]; then
    export PATH=$PATH:$TOOLCHAIN
    echo "Added toolchain to path"
fi

export IDF_PATH=$ESP_IDF

echo "Done"
