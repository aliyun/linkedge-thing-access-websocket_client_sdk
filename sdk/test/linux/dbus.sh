#!/bin/bash
while true
do
echo "set float_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'set' string:'{"params":{"float_rw":11.1}}'
sleep $1
echo "get float_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'get' string:'{"params":["float_rw"]}'

sleep $1
echo "set int32_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'set' string:'{"params":{"int32_rw":11}}'
sleep $1
echo "get int32_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'get' string:'{"params":["int32_rw"]}'

sleep $1
echo "set enum_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'set' string:'{"params":{"enum_rw":2}}'
sleep $1
echo "get enum_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'get' string:'{"params":["enum_rw"]}'

sleep $1
echo "set bool_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'set' string:'{"params":{"bool_rw":1}}'
sleep $1
echo "get bool_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'get' string:'{"params":["bool_rw"]}'
sleep $1

echo "set double_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'set' string:'{"params":{"double_rw":11.1}}'
sleep $1
echo "get double_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'get' string:'{"params":["double_rw"]}'
sleep $1

echo "set string_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'set' string:'{"params":{"string_rw":"hello"}}'
sleep $1
echo "get string_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'get' string:'{"params":["string_rw"]}'
sleep $1

echo "set date_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'set' string:'{"params":{"date_rw":"12345"}}'
sleep $1
echo "get date_rw"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'get' string:'{"params":["date_rw"]}'
sleep $1

echo "call service"
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.ida1vjBTcV2DS_wanglingchao_test001 --print-reply /iot/device/ida1vjBTcV2DS_wanglingchao_test001 iot.device.ida1vjBTcV2DS_wanglingchao_test001.callServices string:'service_write' string:'{"params":{"int32_rw":123, "double_rw":1.23, "float_rw":1.23, "enum_rw":1, "bool_rw":1, "string_rw":"world", "date_rw":"12345678"}}'
sleep $1
done
