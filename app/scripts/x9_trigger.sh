#!/bin/bash
if [ ! -e /sys/class/pwm/pwmchip0/pwm3 ];then
echo 3 > /sys/class/pwm/pwmchip0/export
fi
if [ $# -eq 0 ];then
    Fps=30
else
    Fps=$1
fi
PeriodSet=$((30*33300000/${Fps}))
DutyCycle=$((30*1666000/${Fps}))

echo -e "Fps=${Fps};PeriodSet=${PeriodSet};DutyCycle=${DutyCycle}\n"

echo 0 > /sys/class/pwm/pwmchip0/pwm3/enable
echo ${PeriodSet} > /sys/class/pwm/pwmchip0/pwm3/period
echo ${DutyCycle} > /sys/class/pwm/pwmchip0/pwm3/duty_cycle
echo 1 > /sys/class/pwm/pwmchip0/pwm3/enablel