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
#include "nn_logical.h"
#include "nn.h"

/*
 * nn_lshift_fixedlen: left logical shift in N, i.e. compute out = (in << cnt).
 *
 * Aliasing is possible for 'in' and 'out', i.e. x <<= cnt can be computed
 * using nn_lshift_fixedlen(x, x, cnt).
 *
 * The function supports 'in' and 'out' parameters of differents sizes.
 *
 * The operation time of the function depends on the size of 'in' and
 * 'out' parameters and the value of 'cnt'. It does not depend on the
 * value of 'in'.
 *
 * It is to be noted that the function uses out->wlen as the
 * upper limit for its work, i.e. bits shifted above out->wlen
 * are lost (the NN size of the output is not modified).
 */
void nn_lshift_fixedlen(nn_t out, nn_src_t in, bitcnt_t cnt)
{
	u8 lshift, hshift;
	u8 owlen = out->wlen;
	u8 iwlen = in->wlen;
	int ipos, opos, dec;

	nn_check_initialized(in);
	/* Check that the output is initialized, because we trust its wlen */
	nn_check_initialized(out);

	dec = cnt / WORD_BITS;
	hshift = cnt % WORD_BITS;
	lshift = WORD_BITS - hshift;

	for (opos = owlen - 1; opos >= 0; opos--) {
		word_t hipart = 0, lopart = 0;

		ipos = opos - dec - 1;
		if ((ipos >= 0) && (ipos < iwlen)) {
			lopart = WRSHIFT(in->val[ipos], lshift);
		}

		ipos = opos - dec;
		if ((ipos >= 0) && (ipos < iwlen)) {
			hipart = WLSHIFT(in->val[ipos], hshift);
		}

		out->val[opos] = hipart | lopart;
	}
}

/*
 * nn_lshift: left logical shift in N, i.e. compute out = (in << cnt).
 *
 * Aliasing is possible for 'in' and 'out', i.e. x <<= cnt can be computed
 * using nn_lshift(x, x, cnt).
 *
 * The function supports 'in' and 'out' parameters of differents sizes.
 *
 * The operation time of the function depends on the size of 'in' and
 * 'out' parameters and the value of 'cnt'. It does not depend on the
 * value of 'in'.
 *
 * It is to be noted that the function computes the output bit length
 * depending on the shift count and the input length, i.e. out bit length
 * will be roughly in bit length  plus cnt, maxed to NN_MAX_BIT_LEN.
 */
void nn_lshift(nn_t out, nn_src_t in, bitcnt_t cnt)
{
	u8 owlen, iwlen = in->wlen;
	int ipos, opos, dec;
	u8 lshift, hshift;

	nn_check_initialized(in);
	/* Initialize output if no aliasing is used */
	if (out != in) {
		nn_init(out, 0);
	}

	/* Adapt output length accordingly */
	owlen = (u8)LOCAL_MIN(BIT_LEN_WORDS(cnt + nn_bitlen(in)),
			BIT_LEN_WORDS(NN_MAX_BIT_LEN));
	out->wlen = owlen;

	dec = cnt / WORD_BITS;
	hshift = cnt % WORD_BITS;
	lshift = WORD_BITS - hshift;

	for (opos = owlen - 1; opos >= 0; opos--) {
		word_t hipart = 0, lopart = 0;

		ipos = opos - dec - 1;
		if ((ipos >= 0) && (ipos < iwlen)) {
			lopart = WRSHIFT(in->val[ipos], lshift);
		}

		ipos = opos - dec;
		if ((ipos >= 0) && (ipos < iwlen)) {
			hipart = WLSHIFT(in->val[ipos], hshift);
		}

		out->val[opos] = hipart | lopart;
	}
}

/*
 * nn_rshift_fixedlen: right logical shift in N, i.e. compute out = (in >> cnt).
 *
 * Aliasing is possible for 'in' and 'out', i.e. x >>= cnt can be computed
 * using nn_rshift_fixedlen(x, x, cnt).
 *
 * The function supports 'in' and 'out' parameters of differents sizes.
 *
 * The operation time of the function depends on the size of 'in' and
 * 'out' parameters and the value of 'cnt'. It does not depend on the
 * value of 'in'.
 * It is to be noted that the function uses out->wlen as the
 * upper limit for its work, which means zeroes are shifted in while
 * keeping the same NN output size.
 */
void nn_rshift_fixedlen(nn_t out, nn_src_t in, bitcnt_t cnt)
{
	bitcnt_t lshift, hshift;
	u8 owlen = out->wlen;
	u8 iwlen = in->wlen;
	int ipos, opos, dec;

	nn_check_initialized(in);
	/* Check that the output is initialized, because we trust its wlen */
	nn_check_initialized(out);

	dec = cnt / WORD_BITS;
	lshift = cnt % WORD_BITS;
	hshift = WORD_BITS - lshift;

	for (opos = 0; opos < owlen; opos++) {
		word_t hipart = 0, lopart = 0;

		ipos = opos + dec;
		if ((ipos >= 0) && (ipos < iwlen)) {
			lopart = WRSHIFT(in->val[ipos], lshift);
		}

		ipos = opos + dec + 1;
		if ((ipos >= 0) && (ipos < iwlen)) {
			hipart = WLSHIFT(in->val[ipos], hshift);
		}

		out->val[opos] = hipart | lopart;
	}
}

/*
 * nn_rshift: right logical shift in N, i.e. compute out = (in >> cnt).
 *
 * Aliasing is possible for 'in' and 'out', i.e. x >>= cnt can be computed
 * using nn_rshift_fixedlen(x, x, cnt).
 *
 * The function supports 'in' and 'out' parameters of differents sizes.
 *
 * The operation time of the function depends on the size of 'in' and
 * 'out' parameters and the value of 'cnt'. It does not depend on the
 * value of 'in'.
 * It is to be noted that the function adapts the output size to
 * the input size and the shift bit count, i.e. out bit lenth is roughly
 * equal to input bit length minus cnt.
 */
void nn_rshift(nn_t out, nn_src_t in, bitcnt_t cnt)
{
	u8 owlen, iwlen = in->wlen;
	bitcnt_t lshift, hshift;
	int ipos, opos, dec;

	nn_check_initialized(in);
	/* Initialize output if no aliasing is used */
	if (out != in) {
		nn_init(out, 0);
	}

	dec = cnt / WORD_BITS;
	lshift = cnt % WORD_BITS;
	hshift = WORD_BITS - lshift;

	/* Adapt output length accordingly */
	if (cnt > nn_bitlen(in)) {
		owlen = 0;
	} else {
		owlen = (u8)BIT_LEN_WORDS(nn_bitlen(in) - cnt);
	}
	/* Adapt output length in out */
	out->wlen = owlen;

	for (opos = 0; opos < owlen; opos++) {
		word_t hipart = 0, lopart = 0;

		ipos = opos + dec;
		if ((ipos >= 0) && (ipos < iwlen)) {
			lopart = WRSHIFT(in->val[ipos], lshift);
		}

		ipos = opos + dec + 1;
		if ((ipos >= 0) && (ipos < iwlen)) {
			hipart = WLSHIFT(in->val[ipos], hshift);
		}

		out->val[opos] = hipart | lopart;
	}

	/*
	 * Zero the output upper part now that we don't need it anymore
	 * NB: as we cannot use our normalize helper here (since a consistency
	 * check is done on wlen and upper part), we have to do this manually
	 */
	for (opos = owlen; opos < NN_MAX_WORD_LEN; opos++) {
		out->val[opos] = 0;
	}
}

/*
 * This function right rotates the input NN value by the value 'cnt' on the
 * bitlen basis. The function does it in the following way; right rotation
 * of x by cnt is "simply": (x >> cnt) ^ (x << (bitlen - cnt))
 */
void nn_rrot(nn_t out, nn_src_t in, bitcnt_t cnt, bitcnt_t bitlen)
{
	u8 owlen = (u8)BIT_LEN_WORDS(bitlen);
	nn tmp;

	MUST_HAVE(bitlen <= NN_MAX_BIT_LEN);
	MUST_HAVE(cnt < bitlen);
	nn_check_initialized(in);

	nn_init(&tmp, 0);
	nn_lshift(&tmp, in, bitlen - cnt);
	nn_set_wlen(&tmp, owlen);
	nn_rshift(out, in, cnt);
	nn_set_wlen(out, owlen);
	nn_xor(out, out, &tmp);
	/* Mask the last word if necessary */
	if (((bitlen % WORD_BITS) != 0) && (out->wlen > 0)) {
		/* shift operation below is ok (less than WORD_BITS) */
		word_t mask = ((word_t)(WORD(1) << (bitlen % WORD_BITS))) - 1;
		out->val[out->wlen - 1] &= mask;
	}

	nn_uninit(&tmp);
}

/*
 * This function left rotates the input NN value by the value 'cnt' on the
 * bitlen basis. The function does it in the following way; Left rotation
 * of x by cnt is "simply": (x << cnt) ^ (x >> (bitlen - cnt))
 */
void nn_lrot(nn_t out, nn_src_t in, bitcnt_t cnt, bitcnt_t bitlen)
{
	u8 owlen = (u8)BIT_LEN_WORDS(bitlen);
	nn tmp;

	MUST_HAVE(bitlen <= NN_MAX_BIT_LEN);
	MUST_HAVE(cnt < bitlen);
	nn_check_initialized(in);

	nn_init(&tmp, 0);
	nn_lshift(&tmp, in, cnt);
	nn_set_wlen(&tmp, owlen);
	nn_rshift(out, in, bitlen - cnt);
	nn_set_wlen(out, owlen);
	nn_xor(out, out, &tmp);
	/* Mask the last word if necessary */
	if (((bitlen % WORD_BITS) != 0) && (out->wlen > 0)) {
		word_t mask = ((word_t)(WORD(1) << (bitlen % WORD_BITS))) - 1;
		out->val[out->wlen - 1] &= mask;
	}

	nn_uninit(&tmp);
}

/*
 * Compute XOR between B and C and put the result in A. B and C must be
 * initialized. Aliasing is supported, i.e. A can be one of the parameter B or
 * C. If aliasing is not used, A will be initialized by the function. Function
 * execution time depends on the word length of larger parameter but not on its
 * particular value.
 */
void nn_xor(nn_t A, nn_src_t B, nn_src_t C)
{
	u8 i;

	nn_check_initialized(B);
	nn_check_initialized(C);

	/* Initialize the output if no aliasing is used */
	if ((A != B) && (A != C)) {
		nn_init(A, 0);
	}

	/* Set output wlen accordingly */
	A->wlen = (C->wlen < B->wlen) ? B->wlen : C->wlen;

	for (i = 0; i < A->wlen; i++) {
		A->val[i] = B->val[i] ^ C->val[i];
	}
}

/*
 * Compute logical OR between B and C and put the result in A. B and C must be
 * initialized. Aliasing is supported, i.e. A can be one of the parameter B or
 * C. If aliasing is not used, A will be initialized by the function. Function
 * execution time depends on the word length of larger parameter but not on its
 * particular value.
 */
void nn_or(nn_t A, nn_src_t B, nn_src_t C)
{
	u8 i;

	nn_check_initialized(B);
	nn_check_initialized(C);

	/* Initialize the output if no aliasing is used */
	if ((A != B) && (A != C)) {
		nn_init(A, 0);
	}

	/* Set output wlen accordingly */
	A->wlen = (C->wlen < B->wlen) ? B->wlen : C->wlen;

	for (i = 0; i < A->wlen; i++) {
		A->val[i] = B->val[i] | C->val[i];
	}
}

/*
 * Compute logical AND between B and C and put the result in A. B and C must be
 * initialized. Aliasing is supported, i.e. A can be one of the parameter B or
 * C. If aliasing is not used, A will be initialized by the function. Function
 * execution time depends on the word length of larger parameter but not on its
 * particular value.
 */
void nn_and(nn_t A, nn_src_t B, nn_src_t C)
{
	u8 i;

	nn_check_initialized(B);
	nn_check_initialized(C);

	/* Initialize the output if no aliasing is used */
	if ((A != B) && (A != C)) {
		nn_init(A, 0);
	}

	/* Set output wlen accordingly */
	A->wlen = (C->wlen < B->wlen) ? B->wlen : C->wlen;

	for (i = 0; i < A->wlen; i++) {
		A->val[i] = B->val[i] & C->val[i];
	}
}

/*
 * Compute logical NOT of B and put the result in A. B must be initialized.
 * Aliasing is supported. If aliasing is not used, A will be initialized by
 * the function.
 */
void nn_not(nn_t A, nn_src_t B)
{
	u8 i;

	nn_check_initialized(B);

	/* Initialize the output if no aliasing is used */
	if (A != B) {
		nn_init(A, 0);
	}

	/* Set output wlen accordingly */
	A->wlen = B->wlen;

	for (i = 0; i < A->wlen; i++) {
		A->val[i] = ~(B->val[i]);
	}
}

/* Count leading zeros of a word. This is NOT constant time */
// The below algorithm is taken from Hacker's Delight - Second Edition, 5–3, Counting Leading 0's
static u8 wclz(word_t A)
{
    if (A == 0) return 64;
    u8 c = 0;
    if (A <= 0x00000000FFFFFFFF) { c += 32; A <<= 32; };
    if (A <= 0x0000FFFFFFFFFFFF) { c += 16; A <<= 16; };
    if (A <= 0x00FFFFFFFFFFFFFF) { c += 8; A <<= 8; };
    if (A <= 0x0FFFFFFFFFFFFFFF) { c += 4; A <<= 4; };
    if (A <= 0x3FFFFFFFFFFFFFFF) { c += 2; A <<= 2; };
    if (A <= 0x7FFFFFFFFFFFFFFF) { c += 1; };
    return c;
}

/* Count leading zeros of an initialized nn. This is NOT constant time. */
bitcnt_t nn_clz(nn_src_t in)
{
	bitcnt_t cnt = 0;
	u8 i;

	nn_check_initialized(in);

	for (i = in->wlen; i > 0; i--) {
		if (in->val[i - 1] == 0) {
			cnt += WORD_BITS;
		} else {
			cnt += wclz(in->val[i - 1]);
			break;
		}
	}

	return cnt;
}

/* Compute bit length of given nn. This is NOT constant-time. */
bitcnt_t nn_bitlen(nn_src_t in)
{
	u8 i;

	nn_check_initialized(in);

	for (i = in->wlen; i > 0; i--) {
		if (in->val[i - 1] != 0) {
			return ((i * WORD_BITS) - wclz(in->val[i - 1]));
		}
	}

	return 0;
}

u8 nn_getbit(nn_src_t in, bitcnt_t bit)
{
	bitcnt_t widx = bit / WORD_BITS;
	u8 bidx = bit % WORD_BITS;

	nn_check_initialized(in);
	MUST_HAVE(bit < NN_MAX_BIT_LEN);

	/* bidx is less than WORD_BITS so shift operations below are ok */
	return (u8)((((in->val[widx]) & (WORD(1) << bidx)) >> bidx) & 0x1);
}


