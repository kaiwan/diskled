#!/bin/bash
[ "$1" = "-h" -o "$1" = "--help" ] && {
  echo "Usage: $0 [0|1]
 0 => not verbose [default]
 1 => verbose mode.
 
 Note- Sending SIGUSR1 to the 'led_putios' process will toggle verbosity"
 exit 0
}
verbose=0
if [ $# -eq 1 ] ; then
    [ $1 -eq 1 ] && verbose=1
fi
# 1st param to led_putios is the # of IOs at which the LED is turned On
./led_getios | ./led_putios 2 ${verbose}
