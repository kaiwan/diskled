/*
 * led_getios.c
 *
 * This code gets disk IO statistics in a loop;
 * source: /proc/diskstats ; the field 'ios_in_progress' is the key one
 * that is the output of this program. It will be fed as input to the
 * led_putios program which will interpret it and drive the hardware LED.
 *
 * History:
 *  01Nov2017
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
#include <sys/types.h>

/*---------------- Macros -------------------------------------------*/
#define DISKSTATS_FILE   "/proc/diskstats"

/*---------------- Typedef's, constants, etc ------------------------*/

int r_sleep(time_t sec, long nsec);

/*---------------- Functions ----------------------------------------*/

#define RECMAX   256
#define SEP      '\n'
static int get_iostat(char *prgname)
{
	FILE *fp;
	char rec[RECMAX];
	/* 
	 * Fields of /proc/diskstats are documented here:
	 *  https://www.kernel.org/doc/Documentation/iostats.txt
	 * Vars to hold each field of first line of /proc/diskstats output
	 * Eg. for /dev/sda:
	 *  mj mn devname rd_completed  rd_merged rd_sect    rd_ms      \
	 *  8   0  sda     29376764      778630    660784732 234577452  \
		              wr_completed  wr_merged wr_sect    wr_ms      \
	                   6761746       3970185   168177018 1619243429 \
			          ios_in_progress  ios_ms   ios_ms_weighted     \
					        1         123436977    1853822514
	 */
	unsigned int mj, mn;
	char devname[256]; // chk len ?
	/* The 9 fields documented begin below ... */
	 /* reads: completed, merged, sectors read, ms spent reading */
	unsigned long rd_completed, rd_merged, rd_sect, rd_ms;
	 /* writes: completed, merged, sectors written, ms spent writing */
	unsigned long wr_completed, wr_merged, wr_sect, wr_ms;
	unsigned long ios_in_progress, /* only field that could go to zero;
					all other fields are cumulative (& monotonic?) */
				  ios_ms, ios_ms_weighted;

	fp = fopen(DISKSTATS_FILE, "r");
	if (fp == NULL) {
		fprintf(stderr,"%s: opening %s failed! Aborting...\n", prgname, DISKSTATS_FILE);
		return -1;
	}
	/* We want only the first line of o/p; it's the line summarizing stats for
	 * the disk; eg. for /dev/sda:
	 *  8       0 sda 29376764 778630 660784732 234577452 6761746 3970185 168177018 1619243429 1 123436977 1853822514
	 */
	if ((fgets(rec, RECMAX, fp)) == NULL) {
		fprintf(stderr,"%s: no data!\n", prgname);
		fclose(fp);
		return -1;
	}
	sscanf(rec, "%u %u %s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
			&mj, &mn, devname,
			&rd_completed, &rd_merged, &rd_sect, &rd_ms,
			&wr_completed, &wr_merged, &wr_sect, &wr_ms,
			&ios_in_progress, &ios_ms, &ios_ms_weighted);
#if 0
	printf("rec:ftell=%ld:%s" ftell(fp), rec);
#endif
	//fprintf(stderr, "%lu%c", ios_in_progress, SEP);
	printf("%lu%c", ios_in_progress, SEP);
	fclose(fp);
	return 0;
}

int main(int argc, char **argv)
{
/*	if (argc < 2) {
		fprintf(stderr, "Usage: %s \n", argv[0]);
		exit(EXIT_FAILURE);
	} */
	
	while (1) {
		if (-1 == get_iostat(argv[0]))
			break;
		r_sleep(0,5000000); // s,ns // 5 ms
	//	sleep(1);
	}

	exit (EXIT_SUCCESS);
}

/* vi: ts=4 */
