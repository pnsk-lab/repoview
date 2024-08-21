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
#elif defined(USE_IMAGEMAGICK)
#ifdef __NetBSD__
	printf("-lMagickWand-7.Q16HDRI");
#else
	printf("-lMagickWand-7");
#endif
#endif
	printf("\n");
	return 0;
}
