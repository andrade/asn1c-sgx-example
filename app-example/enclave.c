#include <stdlib.h>

#include <Rectangle.h>

#include <sgx_trts.h>
#include "enclave_t.h"

static int create_and_encode_rectangle(uint8_t *buf, size_t *len, size_t cap)
{
	Rectangle_t *rectangle = NULL; // type to encode
	asn_enc_rval_t rval;           // encoder return value

	rectangle = calloc(1, sizeof(*rectangle));
	if (!rectangle) {
		dump_str("calloc failure for rectangle inside enclave\n");
		return 1;
	}

	rectangle->height = 246;
	rectangle->width = 987;

	rval = der_encode(&asn_DEF_Rectangle, rectangle, NULL, NULL);
	if (1 > rval.encoded) {
		dump_str("Error finding size of Rectangle_t inside enclave\n");
		asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);
		return 1;
	}
	if (rval.encoded > (ssize_t) cap) {
		dump_str("Buffer not large enough for encoding inside enclave\n");
		asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);
		return 1;
	}
	rval = der_encode_to_buffer(&asn_DEF_Rectangle, rectangle, buf, cap);
	if ((ssize_t) -1 == rval.encoded) {
		dump_str("Error DER encoding inside enclave\n");
		asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);
		return 1;
	}
	*len = rval.encoded;

	asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);

	return 0;
}

static int decode_and_print_rectangle(const uint8_t *buf, size_t len)
{
	Rectangle_t *rectangle = 0;    // type to decode
	asn_dec_rval_t rval;           // decoder return value

	rval = ber_decode(0, &asn_DEF_Rectangle, (void **) &rectangle, buf, len);
	if (rval.code != RC_OK) {
		dump_str("ber_decode failure for rectangle inside enclave\n");
		return 1;
	}

	//asn_fprint(stdout, &asn_DEF_Rectangle, rectangle);
	long h = rectangle->height;
	long w = rectangle->width;
	char s[64] = {0};
	snprintf(s, 64, "rectangle received in enclave is (h=%ld,w=%ld)\n", h, w);
	dump_str(s);

	asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);

	return 0;
}

int make_ecall(const void *ibuf, size_t ilen,
		void *obuf, size_t ocap, size_t *olen)
{
	if (decode_and_print_rectangle(ibuf, ilen)) {
		dump_str("Error decoding rectangle inside enclave\n");
		return 1;
	}
	dump_str("decode_and_print_rectangle inside enclave: success\n");

	if (create_and_encode_rectangle(obuf, olen, ocap)) {
		dump_str("Error encoding rectangle inside enclave\n");
		return 1;
	}
	dump_str("create_and_encode_rectangle inside enclave: success\n");

	return 0;
}
