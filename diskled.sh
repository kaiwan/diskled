#!/bin/bash
[ "$1" = "-h" ] && { 
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
./led_getios | ./led_putios 2 ${verbose}
#./led_getios |sudo ./led_putios 2 ${verbose}
