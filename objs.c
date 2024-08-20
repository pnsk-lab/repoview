#include "config.h"

#include <stdio.h>

int main(){
#if defined(USE_SQLITE)
	printf("db/sqlite.o");
#elif defined(USE_GDBM) || defined(USE_NDBM)
	printf("db/dbm.o");
#endif
	printf(" ");
#if defined(USE_MODERN)
	printf("theme/modern.o");
#elif defined(USE_OPTIMIZED)
	printf("theme/optimized.o");
#endif
	printf(" ");
#if defined(USE_COOKIE)
	printf("auth/cookie.o");
#endif
	printf("\n");
	return 0;
}
