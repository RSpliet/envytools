/*
 * Copyright (C) 2011 Marcin Kościelnicki <koriakin@0x04.net>
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nva.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <inttypes.h>

int cnum = 0;
int32_t a;

static const int SZ = 1024 * 1024;

uint32_t queue[1024 * 1024];
volatile int get = 0, put = 0;

#define NV04_PTIMER_TIME_0                                 0x00009400
#define NV04_PTIMER_TIME_1                                 0x00009410

void *watchfun(void *x) {
	while (1) {
		uint32_t nval = nva_rd32(cnum, a);
		if ((nval) != 4) {
			queue[put] = nval;
			put = (put + 1) % SZ;
			nva_wr32(cnum, a, 4); /* Timer resolution is 0x20, 4 is never a
									 valid result */
		}
	}

	return NULL;
}

int main(int argc, char **argv) {
	if (nva_init()) {
		fprintf (stderr, "PCI init failure!\n");
		return 1;
	}
	int c;
	while ((c = getopt (argc, argv, "c:")) != -1)
		switch (c) {
			case 'c':
				sscanf(optarg, "%d", &cnum);
				break;
		}
	if (cnum >= nva_cardsnum) {
		if (nva_cardsnum)
			fprintf (stderr, "No such card.\n");
		else
			fprintf (stderr, "No cards found.\n");
		return 1;
	}

	a = 0x409818;

	pthread_t thr;
	pthread_create(&thr, 0, watchfun, 0);

	while (1) {
		while (get == put)
			sched_yield();

		printf("%d\n", queue[get]);
		get = (get + 1) % SZ;
	}
}
