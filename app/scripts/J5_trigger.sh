#!/bin/bash
PWM_NUM=$(ls /sys/class/pwm/ -v | awk '{print $1}' |head -n 2 |tail -n 1)


#read -p "0.ALL_OFF   1.ALL_ON   " CHOICE
CHOICE="1"

PERIOD=$((33300000 * ${CHOICE}+1000))
DUTY_CYCLE=$((1666000 * ${CHOICE}+1000))
echo "PERIOD=${PERIOD};DUTY_CYCLE=${DUTY_CYCLE}"


cd /sys/class/pwm/${PWM_NUM}/device/pwm/${PWM_NUM}
echo 0 > export
echo 1 > export
echo 2 > export
echo 3 > export

for NUM in {0..3}; do
    cd "pwm${NUM}"
    echo "${PERIOD}" > period
    echo "${DUTY_CYCLE}" > duty_cycle
    cd ..
done 
cd ../..
echo ${CHOICE} > trigger_all

cat  /sys/kernel/debug/pwm

