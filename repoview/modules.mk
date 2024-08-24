mod_repoview.la: mod_repoview.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_repoview.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_repoview.la
