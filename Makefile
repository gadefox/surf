-include config.mk

SURF_CFLAGS += -Wall -std=c99
WEBEXT_CFLAGS += -Wall -std=c99 -fPIC

all: config.mk outdir surf webext.so

config.mk:
	@if ! test -e config.mk; then printf "\033[31;1mERROR:\033[0m you have to run ./configure\n"; exit 1; fi

SURF_OBJ = out/surf/btn.o \
           out/surf/callback.o \
           out/surf/channel.o \
           out/surf/client.o \
           out/surf/encoding.o \
           out/surf/file.o \
           out/surf/jsonrc.o \
           out/surf/key.o \
           out/surf/parameter.o \
           out/surf/serialize.o \
           out/surf/sitespecific.o \
           out/surf/string.o \
           out/surf/surf.o \
           out/surf/verbose.o \
           out/surf/webkit.o

WEBEXT_OBJ = out/webext/encoding.o \
             out/webext/channel.o \
             out/webext/string.o \
             out/webext/verbose.o \
             out/webext/webext.o

$(SURF_OBJ):
	$(QUIET_CC)$(CC) $(CFLAGS) $(SURF_CFLAGS) -c src/$(@F:.o=.c) -o $@

$(WEBEXT_OBJ):
	$(QUIET_CC)$(CC) $(CFLAGS) $(WEBEXT_CFLAGS) -c src/$(@F:.o=.c) -o $@

surf: $(SURF_OBJ)
	$(QUIET_LINK)$(CC) $^ $(SURF_LIBS) -o out/surf/$@

webext.so: $(WEBEXT_OBJ)
	$(QUIET_LINK)$(CC) $^ $(WEBEXT_LIBS) -o out/webext/$@ -shared -Wl,-soname,$@

outdir:
	@mkdir -p out/surf
	@mkdir -p out/webext
    
install:
	@echo installing surf
	@mkdir -p $(DESTDIR)$(BIN_DIR)
	@cp -f out/surf/surf $(DESTDIR)$(BIN_DIR)
	@strip -s $(DESTDIR)$(BIN_DIR)/surf
	@chmod 755 $(DESTDIR)$(BIN_DIR)/surf

	@echo installing surf.rc
	@mkdir -p $(DESTDIR)$(LIBEXEC_DIR)/surf
	@cp -f data/surf.rc $(DESTDIR)$(LIBEXEC_DIR)/surf
	@chmod 755 $(DESTDIR)$(LIBEXEC_DIR)/surf/surf.rc
	
	@echo installing webext
	@mkdir -p $(DESTDIR)$(LIB_DIR)/surf
	@cp -f out/webext/webext.so $(DESTDIR)$(LIB_DIR)/surf
	@strip -s $(DESTDIR)$(LIB_DIR)/surf/webext.so
	@chmod 755 $(DESTDIR)$(LIB_DIR)/surf/webext.so

	@echo installing manual
	@mkdir -p $(DESTDIR)$(MAN_DIR)
	@cp -f data/surf.1 $(DESTDIR)$(MAN_DIR)
	@chmod 644 $(DESTDIR)$(MAN_DIR)/surf.1
	@gzip -f -9 $(DESTDIR)$(MAN_DIR)/surf.1
    
uninstall:
	@echo uninstalling surf
	@rm -f $(DESTDIR)$(BIN_DIR)/surf
	@echo uninstalling surf.rc
	@rm -f $(DESTDIR)$(LIBEXEC_DIR)/surf/surf.rc
	@echo uninstalling webext
	@rm -f $(DESTDIR)$(LIB_DIR)/surf/webext.so
	@echo uninstalling manual
	@rm -f $(DESTDIR)$(MAN_DIR)/surf.1.gz

clean:
	@echo removing surf output files..
	@rm -f out/surf/*.o
	@rm -f out/surf/surf

	@rm -f removing webext output files..
	@rm -f out/webext/*.o
	@rm -f out/webext/webext.so

distclean: clean
	@echo removing config.mk include file
	@rm -f config.mk

.PHONY: all clean distclean install uninstall
