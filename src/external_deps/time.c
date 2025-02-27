/*
 *  Copyright (C) 2017 - This file is part of libecc project
 *
 *  Authors:
 *      Ryad BENADJILA <ryadbenadjila@gmail.com>
 *      Arnaud EBALARD <arnaud.ebalard@ssi.gouv.fr>
 *      Jean-Pierre FLORI <jean-pierre.flori@ssi.gouv.fr>
 *
 *  Contributors:
 *      Nicolas VIVET <nicolas.vivet@ssi.gouv.fr>
 *      Karim KHALFALLAH <karim.khalfallah@ssi.gouv.fr>
 *
 *  This software is licensed under a dual BSD and GPL v2 license.
 *  See LICENSE file at the root folder of the project.
 */

#include "time.h"

#if defined(WITH_CKB)
int get_ms_time(u64 *time) {
  static u64 current_time = 0;
  *time = current_time;
  current_time++;
  return 0;
}

/* Unix and compatible case (including macOS) */
#elif defined(WITH_STDLIB) && (defined(__unix__) || defined(__APPLE__))
#include <stddef.h>
#include <sys/time.h>

int get_ms_time(u64 *time)
{
	struct timeval tv;
	int ret;

	ret = gettimeofday(&tv, NULL);
	if (ret < 0) {
		goto err;
	}
	*time = (u64)(((tv.tv_sec) * 1000) + ((tv.tv_usec) / 1000));

	return 0;
 err:
	return -1;
}

/* Windows case */
#elif defined(WITH_STDLIB) && defined(__WIN32__)
#include <stddef.h>
#include <windows.h>
int get_ms_time(u64 *time)
{
	SYSTEMTIME st;

	GetSystemTime(&st);
	*time = (((st.wMinute * 60) + st.wSecond) * 1000) + st.wMilliseconds;

	return 0;
}

/* No platform detected, the used must provide an implementation! */
#else
#error "time.c: you have to implement get_ms_time()"
#endif
