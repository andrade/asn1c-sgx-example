#include <stdio.h>

#include <sgx_urts.h>
#include "enclave_u.h"

void dump_str(const char *str)
{
	printf("[enclave] %s", str);
}
