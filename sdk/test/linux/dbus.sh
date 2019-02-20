if [ $# != 2 ]; then
    echo "usage: ./dbus.sh [pk] [dn]"
    exit
fi

#valid
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'get' string:'{"params":["power"]}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'get' string:'{"params":["brightness"]}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'set' string:'{"params":{"brightness":2}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'set' string:'{"params":{"power":0}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'blink' string:'{"params":{}}'

#invalid
if false;then
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'set' string:'{"params":{"power":1.234}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'set' string:'{"params":{"power":"1234"}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'set' string:'{"params":{"power":"abc"}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'set' string:'{"params":{"power":["1234"]}}'
dbus-send --bus=unix:path=/tmp/var/run/mbusd/mbusd_socket --dest=iot.device.id$1_$2 --print-reply /iot/device/id$1_$2 iot.device.id$1_$2.callServices string:'set' string:'{"params":{"power":{"key":"value"}}}'
fi
