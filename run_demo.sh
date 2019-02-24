#!/bin/bash

FILENAME=$1

if [${FILENAME} = ""]; then
	FILENAME="test"
fi

if [ -f ${FILENAME}.jpg ]; then
    rm ${FILENAME}.jpg
fi

./get_pic.exp ${FILENAME}; xdg-open ${FILENAME}.jpg

