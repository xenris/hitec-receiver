#!/bin/sh

# Copyright (c) 2017 Henry Shepperd

# MIT License (https://opensource.org/licenses/MIT)

set -e

source ./build.config

if [ $avr_tools ]; then
    if [ -d $avr_tools ]; then
        PATH=$avr_tools/bin:$PATH
        avrdudeconfig="-C $avr_tools/etc/avrdude.conf"
    else
        echo "$avr_tools not found"
        exit 1
    fi
fi

# Clone nbavr if it doesn't exist.
if [ ! -d "lib/nbavr/" ]; then
    git clone https://github.com/xenris/nbavr.git lib/nbavr/
fi

elf="gen/firmware.elf"
hex="gen/firmware.hex"

# Iterate through args.
args=$*
for ((i = 0; i < ${#args}; i++)); do
    arg="${args:$i:1}"

    case "$arg" in
    'b')
        echo "---------------------------------"
        echo "Building"
        echo "---------------------------------"

        tup

        ;;
    'm')
        echo "---------------------------------"

        avr-size $elf -C --mcu=$mcu

        ;;
    'u')
        echo "---------------------------------"
        echo "Uploading"
        echo "---------------------------------"

        if [ -a $upload_port ];
        then
            avrdude -b $upload_baud $avrdudeconfig -p $mcu -P $upload_port -c $programmer -e -U flash:w:$hex
        else
            echo "$upload_port does not exist"
        fi

        ;;
    's')
        echo "---------------------------------"
        echo "Serial - Exit with ctrl+c"
        echo "---------------------------------"

        if [ -a $serial_port ];
        then
            set +e
            trap ' ' INT

            stty -F $serial_port $serial_baud -cstopb -parenb cs8 -echo -icanon raw
            stty -echo -icanon

            exec 3<> $serial_port
            cat <&3 &
            P1=$!
            cat >&3

            kill $P1
            wait $P1 2>/dev/null

            stty echo icanon

            echo ''

            set -e
        else
            echo "$serial_port does not exist"
        fi

        ;;
    'n')
        echo "---------------------------------"
        echo "Updating nbavr"
        echo "---------------------------------"

        cd lib/nbavr/
        git pull
        cd ../../

        ;;
    ' ')
        ;;
    *)
        echo "$arg is not an option"
    esac
done

if [ "$#" -eq "0" ]
then
    echo "b -> Build"
    echo "m -> Show memory usage"
    echo "u -> Upload"
    echo "s -> Serial communication"
    echo "n -> Update nbavr"
    echo ""
    echo "e.g. ./build.sh bmus"
fi
