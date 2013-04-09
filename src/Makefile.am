lib_LTLIBRARIES = sylfm.la

sylfm_la_SOURCES = sylfm.c \
        $(sylplugin_factory_source_dir)/src/sylplugin_factory.c \
        $(sylplugin_factory_source_dir)/src/sylplugin_factory.h

sylfm_la_LDFLAGS = -module -no-undefined -avoid-version

INCLUDES = \
	$(GTK_CFLAGS) \
	$(HTML_CFLAGS) \
	-I$(includedir) \
	-I$(sylpheed_build_dir) \
	-I$(sylpheed_build_dir)/libsylph \
	-I$(sylpheed_build_dir)/src \
	-I$(sylplugin_factory_source_dir)/src

sylfm_la_LIBADD = \
	$(GTK_LIBS) \
	$(sylpheed_build_dir)/src/libsylpheed-plugin-0.la \
	$(sylpheed_build_dir)/libsylph/libsylph-0.la

install:
install-plugin: install-am
