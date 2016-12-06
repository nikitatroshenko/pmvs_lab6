#!/bin/bash
insmod calc_module.ko
`dmesg | tac | grep -m 1 'mknod /dev/left' | awk -F "'" {'print $2'}`
`dmesg | tac | grep -m 1 'mknod /dev/right' | awk -F "'" {'print $2'}`
`dmesg | tac | grep -m 1 'mknod /dev/operator' | awk -F "'" {'print $2'}`

function check {
    echo $1 > /dev/left
    echo "$2" > /dev/operator
    echo $3 > /dev/right
    if [ "$4" == "`cat /proc/result`" ]; then
        echo "True!"
    else
        echo "False!"
    fi
}

check 4 + 6 " 10"
check 18 + 3 " 21"
check 80 + 29 "109"

check 30 - 18 " 12"
check 45 - 15 " 30"
check 3 - 2 "  1"

check 66 / 22 "  3"
check 52 / 13 "  4"
check 105 / 7 " 15"

check 50 "*" 3 "150"
check 21 "*" 3 " 63"
check 37 "*" 3 "111"

rmmod calc_module

rm /dev/operator
rm /dev/left
rm /dev/right
