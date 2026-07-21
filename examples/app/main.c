#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    printf("hello world\n");
END:
    goto END;
    return EXIT_SUCCESS;
}
