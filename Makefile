CC               = gcc
CFLAGS           = -g -O2 -Wall -std=gnu99 -ansi -no-pie
RESOURCES_CC     = glib-compile-resources
RESOURCES_FLAGS  = --generate-source
SRC_DIR          = ./src
DEPS             = $(SRC_DIR)/gui.c
LIBS             = `pkg-config --cflags --libs gtk+-3.0 vte-2.91`
BIN              = php-webserver-gui
VERSION          = 1.0.0
C_SRC            = $(SRC_DIR)/$(BIN).c
H_SRC            = $(SRC_DIR)/$(BIN).h
INSTALL_DIR      = /usr/local/bin
APPS_DIR         = /usr/share/applications
PIXMAPS_DIR      = /usr/share/pixmaps
ICON_FILE        = $(PIXMAPS_DIR)/$(BIN).svg
DESKTOP_FILE     = $(APPS_DIR)/$(BIN).desktop
DEB_BIN_DIR      = ./deb-package/usr/bin
DEB_APPS_DIR     = ./deb-package/usr/share/applications
DEB_PIXMAPS_DIR  = ./deb-package/usr/share/pixmaps
DEB_ICON_FILE    = $(DEB_PIXMAPS_DIR)/$(BIN).svg
DEB_DESKTOP_FILE = $(DEB_APPS_DIR)/$(BIN).desktop
DEB_FILE         = ./$(BIN)-$(VERSION)-1.deb

all: $(BIN) $(C_SRC) $(H_SRC) $(SRC_DIR)/gui.glade

$(BIN): $(C_SRC) $(H_SRC) $(SRC_DIR)/gui.glade
	$(RESOURCES_CC) $(SRC_DIR)/gui.gresource.xml $(RESOURCES_FLAGS)
	$(CC) $(CFLAGS) $(C_SRC) -o $(BIN) $(DEPS) $(LIBS)

install:
	cp $(BIN) $(INSTALL_DIR)
	cp $(SRC_DIR)/icon.svg $(ICON_FILE)
	cp $(SRC_DIR)/$(BIN).desktop $(APPS_DIR)

deb:
	cp $(BIN) $(DEB_BIN_DIR)
	cp $(SRC_DIR)/icon.svg $(DEB_PIXMAPS_DIR)/$(BIN).svg
	cp $(SRC_DIR)/$(BIN).desktop $(DEB_APPS_DIR)
	dpkg-deb --build deb-package
	mv deb-package.deb $(DEB_FILE)

uninstall:
	if [ -s $(ICON_FILE) ]; then rm -f $(ICON_FILE); fi
	if [ -s $(INSTALL_DIR)/$(BIN) ]; then rm -f $(INSTALL_DIR)/$(BIN); fi
	if [ -s $(DESKTOP_FILE) ]; then rm $(DESKTOP_FILE); fi

clean:
	if [ -s $(BIN) ]; then rm $(BIN); fi
	if [ -s $(SRC_DIR)/gui.c ]; then rm $(SRC_DIR)/gui.c; fi
	if [ -s $(DEB_BIN_DIR)/$(BIN) ]; then rm $(DEB_BIN_DIR)/$(BIN); fi
	if [ -s $(DEB_ICON_FILE) ]; then rm $(DEB_ICON_FILE); fi
	if [ -s $(DEB_DESKTOP_FILE) ]; then rm $(DEB_DESKTOP_FILE); fi
	if [ -s $(DEB_FILE) ]; then rm $(DEB_FILE); fi
