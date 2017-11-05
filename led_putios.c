/*
 * led_putios.c
 * Disk Activity Indicator:
 * This code sets / drives a hardware LED on the laptop; it's input is from the
 * led_getios program.
 *
 * History:
 *  02Nov2017
 *
 * Author(s) : 
 * Kaiwan N Billimoria
 *  <kaiwan -at- kaiwantech -dot- com>
 *
 * License(s): MIT permissive
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/*---------------- Macros -------------------------------------------*/
#define PRGNAME    "Disk Activity LED"
#define LED_FILE   "/proc/acpi/ibm/led"
#define ON         1
#define OFF        0

#define LED_TURN_OFF(fd)  do {  \
  __drive_led(fd, OFF);         \
} while(0)
#define LED_TURN_ON(fd)  do {   \
  __drive_led(fd, ON);          \
} while(0)

/*---------------- Function prototypes ------------------------------*/
static void __drive_led(int fd, int state);
int r_sleep(time_t sec, long nsec);

/*---------------- Globals, typedef's, constants, etc ---------------*/
const char buf_on[] = "0 on";
const char buf_off[] = "0 off";
static unsigned int gLed_turnon_threshold = 0;
static int gLedfd, gVerbose = 0;

/*---------------- Functions ----------------------------------------*/

static void user_interrupt(int signum, siginfo_t * siginfo, void *rest)
{
	switch (signum) {
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
	case SIGHUP:
		fprintf(stderr,
			"\n*** [!] %s(%d): aborting due to user interrupt (signal %d) ***\n",
			PRGNAME, getpid(), signum);
		LED_TURN_OFF(gLedfd);
		close(gLedfd);
		exit(2);
	case SIGUSR1:
		fprintf(stderr,
			"\n[*] %s(%d): received signal %d, toggling verbose mode now\n",
			PRGNAME, getpid(), signum);
		if (1 == gVerbose)
			gVerbose = 0;
		else
			gVerbose = 1;
		break;
	}
}

static void init_signals(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = user_interrupt;
	act.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGINT, &act, 0) == -1) {
		perror("sigaction SIGINT");
		exit(1);
	}
	if (sigaction(SIGQUIT, &act, 0) == -1) {
		perror("sigaction SIGQUIT");
		exit(1);
	}
	if (sigaction(SIGTERM, &act, 0) == -1) {
		perror("sigaction SIGQUIT");
		exit(1);
	}
	if (sigaction(SIGHUP, &act, 0) == -1) {
		perror("sigaction SIGHUP");
		exit(1);
	}
	/* Toggle verbosity */
	if (sigaction(SIGUSR1, &act, 0) == -1) {
		perror("sigaction SIGUSR1");
		exit(1);
	}
}

static void __drive_led(int fd, int state)
{
	if (state) {
		write(fd, buf_on, strlen(buf_on));	// "should" succeed
	} else {
		write(fd, buf_off, strlen(buf_off));	// "should" succeed
	}
}

/*
 * This function receives a parameter - the current "number of IOs in progress"
 * It will interpret it and if it's zero, will turn OFF the IBM Thinkpad Power
 * LED (LED #0, accessed via /proc/acpi/ibm/led ;
 *  echo "0 off" > /proc/acpi/ibm/led
 * To turn it On:
 *  echo "0 on" > /proc/acpi/ibm/led
 *).
 * Depending on whether the ios cross a threshold (TBD), we switch the LED on.
 */
static void drive_led(int fd, unsigned long numios)
{
	if (numios == 0)
		__drive_led(fd, OFF);
	else if (numios >= gLed_turnon_threshold)
		__drive_led(fd, ON);
}

#define RECMAX   256

int main(int argc, char **argv)
{
	char rec[RECMAX];
	unsigned long ios_in_progress;

	init_signals();

	if (argc < 2) {
		fprintf(stderr,
			"Usage: %s led_turn_on_threshold [verbose=0|1]\n"
			"1st param : number of IOs in progress threshold at which LED is turned On\n"
			"2nd param == 1 => verbose mode, [def: 0 or !verbose]\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}
	gLed_turnon_threshold = atoi(argv[1]);
	if (argc == 3)
		gVerbose = atoi(argv[2]);

	printf
	    ("[+] %s(%d): initializing ... led_turnon_threshold=%u, verbose=%d\n",
	     PRGNAME, getpid(), gLed_turnon_threshold, gVerbose);

	if ((gLedfd = open(LED_FILE, O_RDWR)) == -1) {
		fprintf(stderr, "[!] %s(%d): open on %s failed, aborting...\n"
		    "Check, does your Linux laptop system have this hardware LED and thus this proc file?\n"
			" (alternate tip: try as superuser, though it should have perms via caps)\n",
			argv[0], getpid(), LED_FILE);
		return 1;
	}
	printf("[+] %s(%d): init done, running ...\n", PRGNAME, getpid());

	while (1) {
		if ((fgets(rec, RECMAX, stdin)) == NULL) {
			fprintf(stderr, "%s(%d): no data!\n", argv[0],
				getpid());
			close(gLedfd);
			return 1;
		}
		sscanf(rec, "%lu", &ios_in_progress);
		if (1 == gVerbose)
			fprintf(stderr, " %lu", ios_in_progress);
		drive_led(gLedfd, ios_in_progress);
		r_sleep(0, 5000000);	// s,ns // 5 ms
	}

	LED_TURN_OFF(gLedfd);
	close(gLedfd);
	exit(EXIT_SUCCESS);
}

/* vi: ts=4 */
