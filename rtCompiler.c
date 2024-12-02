#include <stdio.h>
#include <stdlib.h>

void print_int(int v)
{
    printf("%d\n", v);
}

void print_bool(int v)
{
    printf("%s\n", v ? "true" : "false");
}

int compiler_read(char *s)
{
    char buf[64];
    int val;
    printf("Enter a value for %s: ", s);
    fgets(buf, sizeof(buf), stdin);
    if (EOF == sscanf(buf, "%d", &val))
    {
        printf("Value %s is invalid\n", buf);
        exit(1);
    }
    return val;
}