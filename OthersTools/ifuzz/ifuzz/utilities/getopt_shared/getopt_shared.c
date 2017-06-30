#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int getopt(int argc, char * const argv[],const char *optstring)
{
printf("optstring: %s\n",optstring);
exit(-1);
}

int _init()
{
printf("Initialized loader\n");
return 0;
}
