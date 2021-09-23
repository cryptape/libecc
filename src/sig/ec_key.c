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
#include "ec_key.h"
#include "sig_algs.h"
#include "../curves/curves.h"

/*
 * Check if given private key 'A' has been initialized. Returns 0 on success,
 * -1 on error
 */
int priv_key_check_initialized(const ec_priv_key *A)
{
	int ret = 0;

	MUST_HAVE(!((A == NULL) || (A->magic != PRIV_KEY_MAGIC)), ret, err);

err:
	return ret;
}

/*
 * Same as previous but also verifies that the signature algorithm type does
 * match the one passed using 'sig_type'. Returns 0 on success, -1 on error.
 */
int priv_key_check_initialized_and_type(const ec_priv_key *A,
					 ec_sig_alg_type sig_type)
{
	int ret = 0;

	MUST_HAVE(!((A == NULL) || (A->magic != PRIV_KEY_MAGIC) ||
			(A->key_type != sig_type)), ret, err);

err:
	return ret;
}

/*
 * Import a private key from a buffer with known EC parameters and algorithm
 * Note that no sanity check is performed  by the function to verify key
 * is valid for params. Also note that no deep copy of pointed params is
 * performed. The function returns 0 on success, -1 on error.
 */
int ec_priv_key_import_from_buf(ec_priv_key *priv_key,
				const ec_params *params,
				const u8 *priv_key_buf, u8 priv_key_buf_len,
				ec_sig_alg_type ec_key_alg)
{
	int ret;

	MUST_HAVE(!(priv_key == NULL), ret, err);

	ret = nn_init_from_buf(&(priv_key->x), priv_key_buf, priv_key_buf_len); EG(ret, err);

	/* Set key type and pointer to EC params */
	priv_key->key_type = ec_key_alg;
	priv_key->params = (const ec_params *)params;
	priv_key->magic = PRIV_KEY_MAGIC;

err:
	return ret;
}

/*
 * Export a private key 'priv_key' to a buffer 'priv_key_buf' of length
 * 'priv_key_buf_len'. The function returns 0 on sucess, -1 on error.
 */
int ec_priv_key_export_to_buf(const ec_priv_key *priv_key, u8 *priv_key_buf,
			      u8 priv_key_buf_len)
{
	int ret;
	bitcnt_t blen;

	ret = priv_key_check_initialized(priv_key); EG(ret, err);

	/*
	 * Check that there is enough room to export our private key without
	 * losing information.
	 */
	ret = nn_bitlen(&(priv_key->x), &blen); EG(ret, err);
	MUST_HAVE((8 * (u32)priv_key_buf_len) >= (u32)blen, ret, err);

	/* Export our private key */
	ret = nn_export_to_buf(priv_key_buf, priv_key_buf_len, &(priv_key->x));

err:
	return ret;
}

/*
 * Check if given public key 'A' has been initialized. Returns 0 on success,
 * -1 on error
 */
int pub_key_check_initialized(const ec_pub_key *A)
{
	int ret = 0;

	/* XXX not sure if we should have a must_have here */
	MUST_HAVE(!((A == NULL) || (A->magic != PUB_KEY_MAGIC)), ret, err);

err:
	return ret;
}

/*
 * Same as previous but also verifies that the signature algorithm type does
 * match the one passed using 'sig_type'. Returns 0 on success, -1 on error.
 */
int pub_key_check_initialized_and_type(const ec_pub_key *A,
					ec_sig_alg_type sig_type)
{
	int ret = 0;

	MUST_HAVE(!((A == NULL) || (A->magic != PUB_KEY_MAGIC) ||
			(A->key_type != sig_type)), ret, err);

err:
	return ret;
}

/*
 * Import a public key from a buffer with known EC parameters and algorithm
 * Note that no sanity check is performed by the function to verify key
 * is valid for params. Also note that no deep copy of pointed params is
 * performed. The buffer contains projective point coordinates. The function
 * returns 0 on success, -1 on error.
 */
int ec_pub_key_import_from_buf(ec_pub_key *pub_key, const ec_params *params,
			       const u8 *pub_key_buf, u8 pub_key_buf_len,
			       ec_sig_alg_type ec_key_alg)
{
	int ret, isone;

	MUST_HAVE(((pub_key != NULL) && (params != NULL)), ret, err);

	/* Import the projective point */
	ret = prj_pt_import_from_buf(&(pub_key->y),
				     pub_key_buf, pub_key_buf_len,
				     (ec_shortw_crv_src_t)&(params->ec_curve)); EG(ret, err);

	/* If the cofactor of the curve is not 1, we check that
	 * our public key is indeed in the sub-group generated by
	 * our generator.
	 * NOTE: this is indeed a costly operation, but it is necessary
	 * when we do not trust the public key that is provided, which can
	 * be the case in some protocols.
	 */
	ret = nn_isone(&(params->ec_gen_cofactor), &isone); EG(ret, err);
	if (!isone) {
		if (check_prj_pt_order(&(pub_key->y), &(params->ec_gen_order))) {
			ret = -1;
			goto err;
		}
	}

	/* Set key type and pointer to EC params */
	pub_key->key_type = ec_key_alg;
	pub_key->params = (const ec_params *)params;
	pub_key->magic = PUB_KEY_MAGIC;

err:
	return ret;
}

/*
 * Import a public key from a buffer with known EC parameters and algorithm
 * Note that no sanity check is performed  by the function to verify key
 * is valid for params. Also note that no deep copy of pointed params is
 * performed. The buffer contains affine point coordinates. The function
 * returns 0 on success, -1 on error.
 */
int ec_pub_key_import_from_aff_buf(ec_pub_key *pub_key, const ec_params *params,
			       const u8 *pub_key_buf, u8 pub_key_buf_len,
			       ec_sig_alg_type ec_key_alg)
{
	int ret, isone;

	MUST_HAVE(!((pub_key == NULL) || (params == NULL)), ret, err);

	/* Import the projective point */
	ret = prj_pt_import_from_aff_buf(&(pub_key->y),
				     pub_key_buf, pub_key_buf_len,
				     (ec_shortw_crv_src_t)&(params->ec_curve)); EG(ret, err);

	/* If the cofactor of the curve is not 1, we check that
	 * our public key is indeed in the sub-group generated by
	 * our generator.
	 * NOTE: this is indeed a costly operation, but it is necessary
	 * when we do not trust the public key that is provided, which can
	 * be the case in some protocols.
	 */
	ret = nn_isone(&(params->ec_gen_cofactor), &isone); EG(ret, err);
	if(!isone){
		if(check_prj_pt_order(&(pub_key->y), &(params->ec_gen_order))){
			ret = -1;
			goto err;
		}
	}

	/* Set key type and pointer to EC params */
	pub_key->key_type = ec_key_alg;
	pub_key->params = (const ec_params *)params;
	pub_key->magic = PUB_KEY_MAGIC;

err:
	return ret;
}

/*
 * Export a public key to a projective point buffer. The function returns 0 on
 * success, -1 on error.
 */
int ec_pub_key_export_to_buf(const ec_pub_key *pub_key, u8 *pub_key_buf,
			     u8 pub_key_buf_len)
{
	int ret;

	ret = pub_key_check_initialized(pub_key); EG(ret, err);
	ret = prj_pt_export_to_buf(&(pub_key->y), pub_key_buf, pub_key_buf_len);

err:
	return ret;
}

/*
 * Export a public key to an affine point buffer. The function returns 0 on
 * success, -1 on error.
 */
int ec_pub_key_export_to_aff_buf(const ec_pub_key *pub_key, u8 *pub_key_buf,
			     u8 pub_key_buf_len)
{
	int ret;

	ret = pub_key_check_initialized(pub_key); EG(ret, err);
	ret = prj_pt_export_to_aff_buf(&(pub_key->y), pub_key_buf,
				       pub_key_buf_len);

err:
	return ret;
}

/*
 * Check if given key pair 'A' has been initialized. Returns 0 on success,
 * -1 on error
 */
int key_pair_check_initialized(const ec_key_pair *A)
{
	int ret;

	MUST_HAVE((A != NULL), ret, err);

	ret = priv_key_check_initialized(&A->priv_key); EG(ret, err);
	ret = pub_key_check_initialized(&A->pub_key);

err:
	return ret;
}

/*
 * Same as previous but also verifies that the signature algorithm type does
 * match the one passed using 'sig_type'. Returns 0 on success, -1 on error.
 */
int key_pair_check_initialized_and_type(const ec_key_pair *A,
					 ec_sig_alg_type sig_type)
{
	int ret;

	MUST_HAVE((A != NULL), ret, err);

	ret = priv_key_check_initialized_and_type(&A->priv_key, sig_type); EG(ret, err);
	ret = pub_key_check_initialized_and_type(&A->pub_key, sig_type);

err:
	return ret;
}

/*
 * Import a key pair from a buffer representing the private key. The associated
 * public key is computed from the private key. The function returns 0 on
 * success, -1 on error.
 */
int ec_key_pair_import_from_priv_key_buf(ec_key_pair *kp,
					 const ec_params *params,
					 const u8 *priv_key, u8 priv_key_len,
					 ec_sig_alg_type ec_key_alg)
{
	int ret;

	MUST_HAVE((kp != NULL), ret, err);

	/* Import private key */
	ret = ec_priv_key_import_from_buf(&(kp->priv_key), params, priv_key,
					  priv_key_len, ec_key_alg);  EG(ret, err);
	/* Generate associated public key. */
	ret = init_pubkey_from_privkey(&(kp->pub_key), &(kp->priv_key));

err:
	return ret;
}

/*
 * Import a structured private key to buffer. The structure allows some sanity
 * checks. The function returns 0 on success, -1 on error.
 */
int ec_structured_priv_key_import_from_buf(ec_priv_key *priv_key,
					   const ec_params *params,
					   const u8 *priv_key_buf,
					   u8 priv_key_buf_len,
					   ec_sig_alg_type ec_key_alg)
{
	u8 metadata_len = (3 * sizeof(u8));
	u8 crv_name_len;
	u32 len;
	int ret;

	/* We first pull the metadata, consisting of:
	 *   - One byte = the key type (public or private)
	 *   - One byte = the algorithm type (ECDSA, ECKCDSA, ...)
	 *   - One byte = the curve type (FRP256V1, ...)
	 */
	MUST_HAVE((priv_key_buf != NULL), ret, err);
	MUST_HAVE((priv_key_buf_len > metadata_len), ret, err);
	MUST_HAVE((params != NULL), ret, err);
	MUST_HAVE((params->curve_name != NULL), ret, err);

	/* Pull and check the key type */
	MUST_HAVE((EC_PRIVKEY == priv_key_buf[0]), ret, err);

	/* Pull and check the algorithm type */
	MUST_HAVE((ec_key_alg == priv_key_buf[1]), ret, err);

	/* Pull and check the curve type */
	ret = local_strlen((const char *)params->curve_name, &len); EG(ret, err);
	len += 1;
	MUST_HAVE(len < 256, ret, err);
	crv_name_len = (u8)len;

	ret = ec_check_curve_type_and_name((ec_curve_type) (priv_key_buf[2]),
					params->curve_name, crv_name_len); EG(ret, err);
	ret = ec_priv_key_import_from_buf(priv_key, params,
					  priv_key_buf + metadata_len,
					  priv_key_buf_len - metadata_len,
					  ec_key_alg);

 err:
	return ret;
}

/*
 * Export a structured private key to buffer. The structure allows some sanity
 * checks. The function returns 0 on success, -1 on error.
 */
int ec_structured_priv_key_export_to_buf(const ec_priv_key *priv_key,
					 u8 *priv_key_buf, u8 priv_key_buf_len)
{

	u8 metadata_len = (3 * sizeof(u8));
	const u8 *curve_name;
	u8 curve_name_len;
	u32 len;
	ec_curve_type curve_type;
	int ret;

	ret = priv_key_check_initialized(priv_key); EG(ret, err);

	MUST_HAVE((priv_key_buf != NULL), ret, err);
	MUST_HAVE((priv_key_buf_len > metadata_len), ret, err);
	MUST_HAVE((priv_key->params->curve_name != NULL), ret, err);

	/*
	 * We first put the metadata, consisting on:
	 *   - One byte = the key type (public or private)
	 *   - One byte = the algorithm type (ECDSA, ECKCDSA, ...)
	 *   - One byte = the curve type (FRP256V1, ...)
	 */

	/* Push the key type */
	priv_key_buf[0] = (u8)EC_PRIVKEY;

	/* Push the algorithm type */
	priv_key_buf[1] = (u8)priv_key->key_type;

	/* Push the curve type */
	curve_name = priv_key->params->curve_name;

	ret = local_strlen((const char *)curve_name, &len); EG(ret, err);
	len += 1;
	MUST_HAVE(len < 256, ret, err);
	curve_name_len = (u8)len;

	ret = ec_get_curve_type_by_name(curve_name, curve_name_len, &curve_type); EG(ret, err);
	priv_key_buf[2] = (u8)curve_type;

	/* Push the raw private key buffer */
	ret = ec_priv_key_export_to_buf(priv_key, priv_key_buf + metadata_len,
					priv_key_buf_len - metadata_len);

err:
	return ret;
}

/*
 * Import a structured pub key from buffer. The structure allows some sanity
 * checks. The function returns 0 on success, -1 on error.
 */
int ec_structured_pub_key_import_from_buf(ec_pub_key *pub_key,
					  const ec_params *params,
					  const u8 *pub_key_buf,
					  u8 pub_key_buf_len,
					  ec_sig_alg_type ec_key_alg)
{
	u8 metadata_len = (3 * sizeof(u8));
	u8 crv_name_len;
	u32 len;
	int ret;

	MUST_HAVE((pub_key_buf != NULL), ret, err);
	MUST_HAVE((pub_key_buf_len > metadata_len), ret, err);
	MUST_HAVE((params != NULL), ret, err);
	MUST_HAVE((params->curve_name != NULL), ret, err);

	/*
	 * We first pull the metadata, consisting of:
	 *   - One byte = the key type (public or private)
	 *   - One byte = the algorithm type (ECDSA, ECKCDSA, ...)
	 *   - One byte = the curve type (FRP256V1, ...)
	 */

	/* Pull and check the key type */
	if (EC_PUBKEY != pub_key_buf[0]) {
		ret = -1;
		goto err;
	}

	/* Pull and check the algorithm type */
	if (ec_key_alg != pub_key_buf[1]) {
		ret = -1;
		goto err;
	}

	/* Pull and check the curve type */
	ret = local_strlen((const char *)params->curve_name, &len); EG(ret, err);
	len += 1;
	MUST_HAVE(len < 256, ret, err);
	crv_name_len = (u8)len;

	ret = ec_check_curve_type_and_name((ec_curve_type) (pub_key_buf[2]),
					   params->curve_name, crv_name_len); EG(ret, err);
	ret = ec_pub_key_import_from_buf(pub_key, params,
					 pub_key_buf + metadata_len,
					 pub_key_buf_len - metadata_len,
					 ec_key_alg);

err:
	return ret;
}

/*
 * Export a structured pubate key to buffer. The structure allows some sanity
 * checks. The function returns 0 on success, -1 on error.
 */
int ec_structured_pub_key_export_to_buf(const ec_pub_key *pub_key,
					u8 *pub_key_buf, u8 pub_key_buf_len)
{
	u8 metadata_len = (3 * sizeof(u8));
	const u8 *curve_name;
	u8 curve_name_len;
	u32 len;
	ec_curve_type curve_type;
	int ret;

	ret = pub_key_check_initialized(pub_key); EG(ret, err);

	MUST_HAVE((pub_key_buf != NULL), ret, err);
	MUST_HAVE((pub_key_buf_len > metadata_len), ret, err);
	MUST_HAVE((pub_key->params->curve_name != NULL), ret, err);

	/*
	 * We first put the metadata, consisting of:
	 *   - One byte = the key type (public or private)
	 *   - One byte = the algorithm type (ECDSA, ECKCDSA, ...)
	 *   - One byte = the curve type (FRP256V1, ...)
	 */

	/* Push the key type */
	pub_key_buf[0] = (u8)EC_PUBKEY;

	/* Push the algorithm type */
	pub_key_buf[1] = (u8)pub_key->key_type;

	/* Push the curve type */
	curve_name = pub_key->params->curve_name;

	ret = local_strlen((const char *)curve_name, &len); EG(ret, err);
	len += 1;
	MUST_HAVE(len < 256, ret, err);
	curve_name_len = (u8)len;

	ret = ec_get_curve_type_by_name(curve_name, curve_name_len, &curve_type); EG(ret, err);
	pub_key_buf[2] = (u8)curve_type;

	/* Push the raw pub key buffer */
	ret = ec_pub_key_export_to_buf(pub_key, pub_key_buf + metadata_len,
				       pub_key_buf_len - metadata_len);

err:
	return ret;
}

/*
 * Import a key pair from a structured private key buffer. The structure allows
 * some sanity checks. The function returns 0 on success, -1 on error.
 */
int ec_structured_key_pair_import_from_priv_key_buf(ec_key_pair *kp,
						    const ec_params *params,
						    const u8 *priv_key_buf,
						    u8 priv_key_buf_len,
						    ec_sig_alg_type ec_key_alg)
{
	u8 metadata_len = (3 * sizeof(u8));
	u8 crv_name_len;
	u32 len;
	int ret;

	MUST_HAVE((priv_key_buf != NULL), ret, err);
	MUST_HAVE((priv_key_buf_len > metadata_len), ret, err);
	MUST_HAVE((params != NULL), ret, err);
	MUST_HAVE((params->curve_name != NULL), ret, err);

	/* We first pull the metadata, consisting on:
	 *   - One byte = the key type (public or private)
	 *   - One byte = the algorithm type (ECDSA, ECKCDSA, ...)
	 *   - One byte = the curve type (FRP256V1, ...)
	 */

	/* Pull and check the key type */
	if (EC_PRIVKEY != priv_key_buf[0]) {
		ret = -1;
		goto err;
	}

	/* Pull and check the algorithm type */
	if (ec_key_alg != priv_key_buf[1]) {
		ret = -1;
		goto err;
	}

	/* Pull and check the curve type */
	ret = local_strlen((const char *)params->curve_name, &len); EG(ret, err);
	len += 1;
	MUST_HAVE(len < 256, ret, err);
	crv_name_len = (u8)len;

	ret = ec_check_curve_type_and_name((ec_curve_type) (priv_key_buf[2]),
					params->curve_name, crv_name_len); EG(ret, err);
	ret = ec_key_pair_import_from_priv_key_buf(kp, params,
						   priv_key_buf + metadata_len,
						   priv_key_buf_len -
						   metadata_len, ec_key_alg);

 err:
	return ret;
}

/*
 * Import a key pair from a two structured key buffer (private and public one)
 * The function does not verify the coherency between private and public parts.
 * The function returns 0 on success, -1 on error.
 */
int ec_structured_key_pair_import_from_buf(ec_key_pair *kp,
					   const ec_params *params,
					   const u8 *priv_key_buf,
					   u8 priv_key_buf_len,
					   const u8 *pub_key_buf,
					   u8 pub_key_buf_len,
					   ec_sig_alg_type ec_key_alg)
{
	int ret;

	ret = ec_structured_pub_key_import_from_buf(&kp->pub_key, params,
						    pub_key_buf,
						    pub_key_buf_len,
						    ec_key_alg); EG(ret, err);
	ret = ec_structured_priv_key_import_from_buf(&kp->priv_key, params,
						     priv_key_buf,
						     priv_key_buf_len,
						     ec_key_alg);

err:
	return ret;
}

/*
 * Generate a public/private key pair for given signature algorithm, using
 * given EC params. The function returns 0 on success, -1 on error.
 */
int ec_key_pair_gen(ec_key_pair *kp, const ec_params *params,
		    ec_sig_alg_type ec_key_alg)
{
	int ret;

	MUST_HAVE(!(kp == NULL), ret, err);
	MUST_HAVE(!(params == NULL), ret, err);

	/* Get a random value in ]0,q[ */
	ret = nn_get_random_mod(&(kp->priv_key.x), &(params->ec_gen_order)); EG(ret, err);

	/* Set key type and pointer to EC params for private key */
	kp->priv_key.key_type = ec_key_alg;
	kp->priv_key.params = (const ec_params *)params;
	kp->priv_key.magic = PRIV_KEY_MAGIC;

	/* Call our private key generation function */
	ret = gen_priv_key(&(kp->priv_key)); EG(ret, err);

	/* Generate associated public key. */
	ret = init_pubkey_from_privkey(&(kp->pub_key), &(kp->priv_key));

 err:
	if (ret && (kp != NULL)) {
		kp->priv_key.magic = 0;
		kp->pub_key.magic = 0;
	}
	return ret;
}
