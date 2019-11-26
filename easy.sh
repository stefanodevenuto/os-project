#!/bin/bash

if [ "$1" == "easy" ]; then
        export SO_NUM_G="2"
elif [ "$1" == "hard" ]; then
	export SO_NUM_G="4"
fi
