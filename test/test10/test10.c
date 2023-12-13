#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../Annotations/Annotations.h"

void* MustCall("free0") malloc0(size_t __size)
{
    return malloc(__size);
}

void* MustCall("free1") malloc1(size_t __size)
{
    return malloc(__size);
}

void* MustCall("free2") malloc2(size_t __size)
{
    return malloc(__size);
}

void* MustCall("free3") malloc3(size_t __size)
{
    return malloc(__size);
}

void free0(void* p Calls("free0"))
{
    free(p);
}

void free1(void* p Calls("free1"))
{
    free(p);
}

void free2(void* p Calls("free2"))
{
    free(p);
}

void free3(void* p Calls("free3"))
{
    free(p);
}

int main ()
{
    char *str;
    int a = getchar();

    str = (char *) malloc(15);

    do {
        free1(str);

        switch (a) {
        case -15:
            free(str);
            break;
        case -10:
            free(str);
            break;
        default:
            free(str);
            break;
        }
    } while (a > 15);

    return(0);


}

/*
Results:
do.end str {free, free1}

*/