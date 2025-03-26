This is a C hello world for ASN.1 inside SGX.

The example uses the rectangle from the original documentation. The code:

1. Creates a rectangle outside the enclave, DER encodes it, and sends it into the enclave using the `make_ecall` ecall defined in `enclaved.edl`.

2. DER decodes de rectangle inside the enclave and prints its contents.

3. Creates a rectangle inside the enclave, DER encodes it, and sends it out of the enclave.

4. DER decodes the rectangle outside the enclave and prints its contents.

The encoding and decoding functions outside the enclave in `mainapp.c` and inside the enclave in `enclave.c` have small differences because `printf` and `fprintf` are not available in the enclave (no point in implementing for this example). But the ASN.1 blocks are the same.

---

Compile and run with:

```
$ make genkey
$ make

$ ./app_ex
sgx_create_enclave: success
create_and_encode_rectangle: success with length 9
[enclave] rectangle received in enclave is (h=123,w=567)
[enclave] decode_and_print_rectangle inside enclave: success
[enclave] create_and_encode_rectangle inside enclave: success
outside the enclave, make_ecall returned with result 0
Rectangle ::= {
    height: 246
    width: 987
}
decode_and_print_rectangle: success with length 10
sgx_destroy_enclave: success
```

Cleanup with:

```
$ make clean
$ make delkey
```

---

A private key is needed to build the `enclave.signed.so`. The key is generated separately from the source code compilation (the `$ make genkey|delkey`).

The Makefile has simulation mode hardcoded.

Setup the SGX SDK beforehand. If done so correctly, the `SGX_SDK` environment variable should be set and the `sgx_edger8r` and `sgx_sign` should be available. Additionally, set the `ASN_SGX_HOME` environment variable to the location of the ASN.1 static libraries (which for this example is simply the directory above, so `$ export ASN_SGX_HOME=..`).
