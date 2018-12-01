
.PHONY: dist install

all:
	@echo "Pick a target... install to copy from dist/ to system, dist to copy from system to dist/"

dist:
	cp /usr/local/bin/midi_connect.sh ./dist/usr/local/bin/
	cp /etc/udev/rules.d/95-midi.rules ./dist/etc/udev/rules.d/

install:
	echo

