/* $Id$ */

#include "config.h"

#include <stdio.h>

int main() {
#if defined(USE_SQLITE)
	printf("-lsqlite3");
#elif defined(USE_GDBM)
	printf("-lgdbm -lgdbm_compat");
#endif
	printf(" ");
#if defined(USE_GRAPHICSMAGICK)
	printf("-lGraphicsMagickWand");
#endif
	printf(" ");
#if defined(USE_LIBPNG)
	printf("-lpng16");
#endif
	printf("\n");
	return 0;
}
