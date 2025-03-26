CC := gcc
SGX_EDGER ?= sgx_edger8r
SGX_SIGNER ?= sgx_sign

SGX_SDK ?= $(HOME)/.local/lib/x86_64-linux-gnu/sgxsdk

$(info [ CURDIR ] $(CURDIR))
$(info [ SGX_SDK] $(SGX_SDK))

ifndef ASN_SGX_HOME
$(error ASN_SGX_HOME is not set)
endif
$(info [ASN_SGX_HOME] $(ASN_SGX_HOME))

$(info ------------------------------------------------------------------------)

CFLAGS := -m64 -std=c17 -O0 -g -Wall -Wextra

ENC_CPPFLAGS := \
		-D ENABLE_ENCLAVE_ASNONE_CODE=1 \
		-I$(ASN_SGX_HOME)/. -I$(ASN_SGX_HOME)/asn1c/skeletons -I$(ASN_SGX_HOME)/linux -I$(ASN_SGX_HOME)/sgx \
		-I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc
ENC_CFLAGS := $(CFLAGS) \
		-Wno-implicit-function-declaration -nostdinc -fvisibility=hidden -fpie -fstack-protector -fno-builtin-printf
ENC_LDFLAGS := \
		-L$(ASN_SGX_HOME) \
		-Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_SDK)/lib64 -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined -Wl,-pie,-eenclave_entry -Wl,--export-dynamic -Wl,--defsym,__ImageBase=0 -Wl,--version-script=enclave.lds \
		-Wl,-z,noexecstack
ENC_LDLIBS := \
		-lasn1proto-secure \
		-Wl,--whole-archive -lsgx_trts_sim -Wl,--no-whole-archive -Wl,--start-group -lsgx_tstdc -lsgx_tcrypto -lsgx_tservice_sim -Wl,--end-group

APP_CPPFLAGS := \
		-D ENABLE_ENCLAVE_ASNONE_CODE=0 \
		-I$(ASN_SGX_HOME)/. -I$(ASN_SGX_HOME)/asn1c/skeletons \
		-I$(SGX_SDK)/include
APP_CFLAGS := $(CFLAGS) \
		-DDEBUG -UNDEBUG -UEDEBUG \
		-fPIC -Wno-attributes
APP_LDFLAGS := \
		-L$(ASN_SGX_HOME) \
		-L$(SGX_SDK)/lib64 -Wl,-rpath,$(SGX_SDK)/sdk_libs,--disable-new-dtags
APP_LDLIBS := \
		-lasn1proto-normal \
		-lsgx_urts_sim -lsgx_uae_service_sim \
		-lc


all: app_ex
	$(info [  MAKE  ] $(notdir $^))

################################################################
################################ MANAGEMENT
################################################################

.PHONY: clean

################################################################
################################ CRYPTO
################################################################

KEY_PR := key.pem

genkey:
	@openssl genrsa -3 -out $(KEY_PR) 3072
	$(info [ GENKEY ] $^  >  $@)
	$(info [ GENKEY ] Enclave private key is left untouched by clean.)
	$(info [ GENKEY ] Explicitly use delkey to delete it.)

delkey:
	@rm -f $(KEY_PR)
	$(info [ DELKEY ] $^  >  $@)

$(KEY_PR):
	$(info [ KEY_PR ] Create private key with "make genkey")
	$(info [ KEY_PR ] Delete private key with "make delkey")

################################################################
################################ TRUSTED CODE
################################################################

enclave_t.c enclave_t.h: enclave.edl
	@$(SGX_EDGER) --trusted enclave.edl
	$(info [  GEN   ] $^  >  $@)

enclave_t.o: enclave_t.c
	@$(CC) $(ENC_CPPFLAGS) $(ENC_CFLAGS) -c $< -o $@
	$(info [   CC   ] $^  >  $@)

enclave.o: enclave.c enclave_t.o
	@ gcc $(ENC_CPPFLAGS) $(ENC_CFLAGS) -c $< -o $@
	$(info [   CC   ] $^  >  $@)

enclave.so: enclave_t.o enclave.o
	@ gcc $(ENC_LDFLAGS) $^ -o $@ $(ENC_LDLIBS)
	$(info [  GEN   ] $^  >  $@)

enclave.signed.so: enclave.so \
		enclave.config.xml $(KEY_PR)
	@$(SGX_SIGNER) sign -key $(KEY_PR) -enclave enclave.so -out enclave.signed.so -config enclave.config.xml
	$(info [  SIGN  ] $^  >  $@)

################################################################
################################ UNTRUSTED CODE
################################################################

enclave_u.c enclave_u.h: enclave.edl
	@$(SGX_EDGER) --untrusted $<
	$(info [  GEN   ] $^  >  $@)

enclave_u.o: enclave_u.c enclave.signed.so
	@$(CC) $(APP_CPPFLAGS) $(APP_CFLAGS) -c $< -o $@
	$(info [   CC   ] $^  >  $@)

support.o: support.c enclave_u.o
	@ gcc $(APP_CPPFLAGS) $(APP_CFLAGS) -c $< -o $@
	$(info [   CC   ] $^  >  $@)

mainapp.o: mainapp.c enclave_u.h
	@ gcc $(APP_CPPFLAGS) $(APP_CFLAGS) -c $< -o $@
	$(info [   CC   ] $^  >  $@)

app_ex: enclave_u.o support.o mainapp.o
	@$(CC) $(APP_CPPFLAGS) $(APP_CFLAGS) $(APP_LDFLAGS) -o $@ $^ $(APP_LDLIBS)
	$(info [  LINK  ] $^  >  $@)

################################################################
################################ MAINTENANCE
################################################################

TRASH := app_ex \
		enclave_t.h enclave_t.c enclave_t.o \
		enclave.o enclave.so enclave.signed.so \
		enclave_u.h enclave_u.c enclave_u.o \
		support.o mainapp.o
clean:
	@rm -rf $(TRASH)
	$(info [   RM   ] $(TRASH))
