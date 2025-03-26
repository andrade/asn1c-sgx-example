#define _POSIX_C_SOURCE 201710L

#include <stdio.h>

#include <Rectangle.h>

#include <sgx_urts.h>

#include "enclave_u.h"

#define ENCLAVE_FILE "enclave.signed.so"

static int create_and_encode_rectangle(uint8_t *buf, size_t *len, size_t cap)
{
	Rectangle_t *rectangle = NULL; // type to encode
	asn_enc_rval_t rval;           // encoder return value

	rectangle = calloc(1, sizeof(*rectangle));
	if (!rectangle) {
		fprintf(stderr, "calloc failure for rectangle\n");
		return 1;
	}

	rectangle->height = 123;
	rectangle->width = 567;

	rval = der_encode(&asn_DEF_Rectangle, rectangle, NULL, NULL);
	if (1 > rval.encoded) {
		fprintf(stderr, "Error finding size of Rectangle_t\n");
		asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);
		return 1;
	}
	if (rval.encoded > (ssize_t) cap) {
		fprintf(stderr, "Buffer not large enough for encoding (need %zd)\n", rval.encoded);
		asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);
		return 1;
	}
	rval = der_encode_to_buffer(&asn_DEF_Rectangle, rectangle, buf, cap);
	if ((ssize_t) -1 == rval.encoded) {
		fprintf(stderr, "Error DER encoding\n");
		asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);
		return 1;
	}
	*len = rval.encoded;
	//fprintf(stdout, "Encoded Rectangle_t has size = %zd\n", rval.encoded);

	asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);

	return 0;
}

static int decode_and_print_rectangle(const uint8_t *buf, size_t len)
{
	Rectangle_t *rectangle = 0;    // type to decode
	asn_dec_rval_t rval;           // decoder return value

	rval = ber_decode(0, &asn_DEF_Rectangle, (void **) &rectangle, buf, len);
	if (rval.code != RC_OK) {
		fprintf(stderr, "ber_decode failure for rectangle (consumed %zu bytes)\n", rval.consumed);
		return 1;
	}

	asn_fprint(stdout, &asn_DEF_Rectangle, rectangle);

	asn_DEF_Rectangle.free_struct(&asn_DEF_Rectangle, rectangle, 0);

	return 0;
}

int main(void)
{
	sgx_launch_token_t token = {0};
	int updated = 0;
	sgx_enclave_id_t eid;
	sgx_status_t status = SGX_ERROR_UNEXPECTED; // =0x0001

	int result = 0;

	size_t cap = 64;
	uint8_t buf[64] = {0};
	size_t len = 0;

	status = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG,
			&token, &updated, &eid, NULL);
	if (SGX_SUCCESS != status) {
		printf("sgx_create_enclave: failure with error 0x%04x\n", status);
		return EXIT_FAILURE;
	}
	printf("sgx_create_enclave: success\n");

	if (create_and_encode_rectangle(buf, &len, sizeof buf)) {
		fprintf(stderr, "Error encoding rectangle outside enclave\n");
		sgx_destroy_enclave(eid);
		return EXIT_FAILURE;
	}
	printf("create_and_encode_rectangle: success with length %zu\n", len);

	status = make_ecall(eid, &result, buf, len, buf, cap, &len);
	if (SGX_SUCCESS != status) {
		printf("make_ecall: failure with error 0x%04x\n", status);
		sgx_destroy_enclave(eid);
		return EXIT_FAILURE;
	}
	printf("outside the enclave, make_ecall returned with result %d\n", result);

	if (decode_and_print_rectangle(buf, len)) {
		fprintf(stderr, "Error decoding rectangle outside enclave\n");
		sgx_destroy_enclave(eid);
		return EXIT_FAILURE;
	}
	printf("decode_and_print_rectangle: success with length %zu\n", len);

	if (SGX_SUCCESS != sgx_destroy_enclave(eid)) {
		printf("sgx_destroy_enclave: failure with error 0x%04x\n", status);
		return EXIT_FAILURE;
	}
	printf("sgx_destroy_enclave: success\n");

	return EXIT_SUCCESS;
}
