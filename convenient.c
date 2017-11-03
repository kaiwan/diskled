/*
 * convenient.c
 *
 * A few convenience routines..
 * These are the user-space "library" / "convenience" routines.
 *
 * Author: Kaiwan N Billimoria <kaiwan@kaiwantech.com>
 * MIT License
 */
#ifndef __KERNEL__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

/* Subtract the `struct timeval' values X and Y,
    storing the result in RESULT.
    Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *x,
		     struct timeval *y)
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	   tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

/*
 * Converts decimal to binary. 
 * Credits: vegaseat. URL: http://www.daniweb.com/software-development/c/code/216349
 * accepts a decimal integer and returns a binary coded string
 *
 * @decimal : decimal value to convert to binary (IN)
 * @binary  : the binary result as a string (OUT)
 *
 */
void dec2bin(long decimal, char *binary)
{
	int k = 0, n = 0;
	int neg_flag = 0;
	int remain;
	/*
	   gcc 4.6.3 : we get the warning:
	   "warning: variable ‘old_decimal’ set but not used [-Wunused-but-set-variable]"
	   To get rid of this warning, have #ifdef'd the test... -kaiwan.

	   Keep one of the following below (wrt TESTMODE); comment out the other.
	   UN-defining by default.
	 */
//#define TESTMODE
#undef TESTMODE

#ifdef TESTMODE
	int old_decimal;	// for test
#endif
	char temp[80];

	// take care of negative input
	if (decimal < 0) {
		decimal = -decimal;
		neg_flag = 1;
	}
	do {
#ifdef TESTMODE
		old_decimal = decimal;	// for test
#endif
		remain = decimal % 2;
		// whittle down the decimal number
		decimal = decimal / 2;
		// this is a test to show the action
#ifdef TESTMODE
		printf("%d/2 = %d  remainder = %d\n", old_decimal, decimal,
		       remain);
#endif
		// converts digit 0 or 1 to character '0' or '1'
		temp[k++] = remain + '0';
	} while (decimal > 0);

	if (neg_flag)
		temp[k++] = '-';	// add - sign
	else
		temp[k++] = ' ';	// space

	// reverse the spelling
	while (k >= 0)
		binary[n++] = temp[--k];

	binary[n - 1] = 0;	// end with NULL
}

/* 
 * function r _ s l e e p
 *
 * Wrapper around nanosleep(2) , and in such a way that interruption 
 * due to a non-blocked signals causes restart.
 */
#ifndef _TIME_H
#include <time.h>
#endif
int r_sleep(time_t sec, long nsec)
{
	struct timespec req, rem;

	req.tv_sec = sec;
	req.tv_nsec = nsec;
	while (nanosleep(&req, &rem) == -1) {
		if (errno != EINTR)
			return -1;
		/* Actually should not use [f]printf in sig handler - not aysnc-signal 
		   safe - just shown for demo purpose here (works with DEBUG option on).
		   Ugly!
		 */
#ifdef DEBUG
		fprintf(stderr,
			"* nanosleep interrupted! rem time: %lu.%lu *\n",
			rem.tv_sec, rem.tv_nsec);
#endif
		req = rem;
	}
	return 0;
}

/*--------------- Sourced from:
http://www.alexonlinux.com/hex-dump-functions
All rights rest with original author(s).----------------------

Added a 'verbose' parameter..(kaiwan).
*/
void hex_dump(unsigned char *data, int size, char *caption, int verbose)
{
	int i;			// index in data...
	int j;			// index in line...
	char temp[8];
	char buffer[128];
	char *ascii;

	memset(buffer, 0, 128);

	if (verbose && caption)
		printf("---------> %s <--------- (%d bytes from %p)\n", caption,
		       size, data);

	// Printing the ruler...
	printf
	    ("        +0          +4          +8          +c            0   4   8   c   \n");

	// Hex portion of the line is 8 (the padding) + 3 * 16 = 52 chars long
	// We add another four bytes padding and place the ASCII version...
	ascii = buffer + 58;
	memset(buffer, ' ', 58 + 16);
	buffer[58 + 16] = '\n';
	buffer[58 + 17] = '\0';
	buffer[0] = '+';
	buffer[1] = '0';
	buffer[2] = '0';
	buffer[3] = '0';
	buffer[4] = '0';
	for (i = 0, j = 0; i < size; i++, j++) {
		if (j == 16) {
			printf("%s", buffer);
			memset(buffer, ' ', 58 + 16);

			sprintf(temp, "+%04x", i);
			memcpy(buffer, temp, 5);

			j = 0;
		}

		sprintf(temp, "%02x", 0xff & data[i]);
		memcpy(buffer + 8 + (j * 3), temp, 2);
		if ((data[i] > 31) && (data[i] < 127))
			ascii[j] = data[i];
		else
			ascii[j] = '.';
	}

	if (j != 0)
		printf("%s", buffer);
}

int err_exit(char *prg, char *err, int exitcode)
{
#define ERRSTRMAX 512
	char err_str[ERRSTRMAX];

	if (strlen(err) >= ERRSTRMAX)
		err[ERRSTRMAX] = '\0';	// truncate it!

	snprintf(err_str, ERRSTRMAX - 1, "%s: %s", prg, err);
	perror(err_str);

	if (-1 == exitcode)
		return 0;
	exit(exitcode);
}				// err_exit()

#endif
