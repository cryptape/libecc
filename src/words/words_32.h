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
#ifndef __WORDS_32_H__
#define __WORDS_32_H__

/*
 * Types for 32-bit long words and a few useful macros.
 */

#include "types.h"

typedef uint32_t word_t;
typedef uint16_t hword_t;
typedef uint64_t dword_t;

/* WORD_BITS (resp. WORD_BYTES): number of bits (resp. bytes) in a word. */
#define WORD_BITS (32)
#define WORD_BYTES (WORD_BITS / 8)
#define HWORD_BITS (16)
#define HWORD_BYTES (HWORD_BITS / 8)

/* WORD: constant of word-size. */
#define WORD(A) (UINT32_C(A))
#define HWORD(A) (UINT16_C(A))

/* WORD_MAX: maximal value of a word. */
#define WORD_MAX UINT32_MAX
#define HWORD_MAX UINT16_MAX

/* PRINTF_WORD_HEX_FMT: printf hex format string for word */
#ifndef PRIx16
#define PRIx16 "hx"
#endif
#ifndef PRIx32
#define PRIx32 "x"
#endif
#ifndef PRIx64
#define PRIx64 "llx"
#endif
#define PRINTF_WORD_HEX_FMT "%08" PRIx32

#endif /* __WORDS_32_H__ */
