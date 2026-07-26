/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    printf("hello world\n");
END:
    goto END;
    return EXIT_SUCCESS;
}
