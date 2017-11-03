# diskled

**1-line Summary:**
Some laptops don't have a disk activity LED, and that just won't do! 
This project aims to reuse an existing LED as a disk activity indicator.

***NOTE !***
Specifically coded to support a **Lenovo (IBM) Thinkpad T460** laptop; we use the Power LED \#0 as the disk activity LED.

I got the idea when reading a reddit post talking about similar stuff [3].

We'd like to simulate this using software, and an LED on the laptop that we
do have control over (eg., the Power Led #0 over the procfs interface; details below).


**A REQUEST**
Pl try this on a similar laptop and let me know if it works, or what your
experience is.


**HOWTO: TRY IT**

1. git clone  (or git pull)
2. cd [diskled-folder]
3. make
4. ./diskled.sh


**Theory of Operation**

We require a source for disk IO activity; we use */proc/diskstats* .
Eg.

    $ cat /proc/diskstats
       8       0 sda 40565749 1001718 918480123 318424600 12437524 6232361 260453890 2635848839 **4** 193643664 2954393124
       8       1 sda1 554 774 12711 7291 2 0 2 196 0 6616 7487
       8       2 sda2 145 0 12432 2659 0 0 0 0 0 2192 2659
    [...]
    $

The first line is all we capture; the fields in the first line are:

	Device major#, device minor#, device name, 
	reads: completed, merged, sectors read, ms spent reading, 
	writes: completed, merged, sectors written, ms spent writing, 
	**ios_in_progress**, ios ms total, ios ms weighted.
*Details: see [1]*

The important field for us is the **"ios_in_progress"**; it's the only field that is not cumulative and could go to zero. It's an indication of the current
number of IOs in progress. So, we use it to determine whether to turn On or
Off the LED; the way it's currently setup:
the 1st parameter to *led_putios* is the *"led_turnon_threshold"*; we use the
 value 2 as of now..

The program is built architected as a **producer-consumer** model:
     producer | consumer
- led_getios : the producer: this 'C' code fetches and isolates the
  'ios_in_progress' integer, and writes it to stdout followed by a newline '\n'.
- led_putios : the consumer: this 'C' code reads from stdin - thus fetching the
  output of the producer (the IOs in progress) as it's on the RHS of the pipe -
  and will switch On/Off the LED based on the value it reads.

**Tips:**
- to see the actual value of the key variable "ios_in_progress", you can modify
  the wrapper shell script to run led_putios in verbose mode.
  Alternatively, and even better, sending SIGUSR1 to the led_putios process (it's
  PID is displayed), will cause it to toggle the verbosity!
  Careful- don't do this when it's running in the background, else redirect
  it's stderr appropriately.

>Ref:
[1] https://www.kernel.org/doc/Documentation/iostats.txt
[2] https://www.kernel.org/doc/Documentation/laptops/thinkpad-acpi.txt
[3] https://www.reddit.com/r/linux/comments/25teew/turn_your_scroll_lock_into_disk_activity_indicator/

*Written with [StackEdit](https://stackedit.io/).*
