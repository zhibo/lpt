#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

int main()
{
	struct rlimit rlim;
	/* set coredump */
	system("sysctl -n -e -q -w fs.suid_dumpable=2");
	system("sysctl -n -e -q -w kernel.core_pattern=/tmp/core-%e-%p-%u-%g-%s");
	rlim.rlim_cur = RLIM_INFINITY;
	rlim.rlim_max = rlim.rlim_cur;
	setrlimit(RLIMIT_CORE, &rlim);

    char *a = malloc(10);
    strncpy(a, "hello", 10);
    printf("a: %s\n", a);
    free(a);
    free(a);
    return 0;
}
