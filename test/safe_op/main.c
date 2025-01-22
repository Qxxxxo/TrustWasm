/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "safe_op.h"

int
test_hello2(uint32_t buf, size_t buflen);

int
main(int argc, char **argv)
{
    char *buf;
    size_t buflen;
    int res;
    buflen = 1;
    buf = (char *)safe_malloc(5 * sizeof(char));

    uint32_t buf_address = (uint32_t)buf;
    uint32_t buf_address_signed = setSignatureAndTag(buf_address, 127, 0);

    res = test_hello2((uint32_t)buf_address_signed, buflen);
    if (res == -1) {
        return -1;
    }

    if(san_guard(buf, 5)){
        printf("%s\n", buf);
    }

    safe_free(buf, 5);

    return 0;
}
