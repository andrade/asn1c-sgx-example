### Guidelines

#### Compiling

1. Write the `.asn1` specifications and place them in the top directory (where this README is located);

2. Use `$ make validate` to check the specifications for correctness;

3. Use `$ make generate` to generate the ASN.1 tables (output source and header files) based on the specifications and an existing copy of the skeleton files (of version 0.9.28) in the `asn1c/skeletons/` subdirectory; and

4. Use `$ make` to generate a static library named `$(TARGET)` based on the generated ASN.1 tables and skeleton files.

#### Using outside the enclave

Assume the root of this library is at `$(ASN_SGX_HOME)`.

1. Specify the path to the header files with `-I$(ASN_SGX_HOME)/. -I$(ASN_SGX_HOME)/asn1c/skeletons`.

2. Provide the linker with the path to the library with `-L$(ASN_SGX_HOME)`.

3. Provide the linker with the name of the library with `-lasn1proto-normal`.

#### Using inside the enclave

Assume the root of this library is at `$(ASN_SGX_HOME)`.

1. Specify the path to the header files with `-I$(ASN_SGX_HOME)/. -I$(ASN_SGX_HOME)/asn1c/skeletons -I$(ASN_SGX_HOME)/linux -I$(ASN_SGX_HOME)/sgx`.

2. Provide the linker with the path to the library with `-L$(ASN_SGX_HOME)`.

3. Provide the linker with the name of the library with `-lasn1proto-secure`.

4. Add `-D ENABLE_ENCLAVE_ASNONE_CODE=1` to the `CPPFLAGS`, otherwise compilation with fail complaining `netinet/in.h` does not exist.

#### Notes

- *Note #1:* Generate the files before calling make to create the static library. This step is not done automatically.

- *Note #2:* Worth mentioning one can also invoke make as `$ make NORMAL_TARGET=libasn1.a` to set the name of the output library. There is also a `SECURE_TARGET` variable for the enclave version of the library.

- *Note #3:* The generated files can be placed under version control. In such case it is no longer necessary to have the `asn1c` compiler available in other machines using this code (since the necessary support files are available under `asn1c/`). If the compiler is needed, set `ASN1C` to the correct location, for example, `ASN1C=/home/...../asn1c`; otherwise it uses the `asn1c` in path by default.

- *Note #5:* The secure version of the library requires the SGX SDK to be available at `SGX_SDK`. This is only used for the includes at `$(SGX_SDK)/include`, when compiling.

### Known Issues

#### Large integers

The compiler cannot handle large integers.

If we specify `var INTEGER(0..MAX)` it compiles but then fails for the cases where the integer is too large.

Explicitly specifying the max integer value, however, gives us an error directly from the compiler during validation. This is done with `var INTEGER(0..18446744073709551615)`, in this case to store a `uint64_t` which the compiler cannot handle. The error in this case would be `Value "18446744073709551615" at line 11 is too large for this compiler! Please contact the asn1c author.`.
