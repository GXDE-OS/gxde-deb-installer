#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com
/usr/lib/qt6/bin/lupdate -recursive . -ts translations/*.ts
ts_list=(`ls translations/*.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    #lupdate "${ts}"
    /usr/lib/qt6/bin/lrelease "${ts}"
done
