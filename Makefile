
.PHONY: dist install

all:
	@echo "Pick a target... install to copy from dist/ to system, dist to copy from system to dist/"

dist:
	touch ./dist/var/log/midi-server.log
	cp /usr/local/bin/midi_connect.sh          ./dist/usr/local/bin/
	cp /usr/local/bin/midi_disconnect.sh       ./dist/usr/local/bin/
	cp /etc/udev/rules.d/95-midi.rules         ./dist/etc/udev/rules.d/
	cp /etc/systemd/system/midibus.service     ./dist/etc/systemd/system/
	cp /etc/systemd/system/midi-server.service ./dist/etc/systemd/system/

install:
	cp -R dist/{etc,usr,var} /

