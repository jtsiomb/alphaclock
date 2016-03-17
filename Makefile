# change this to install elsewhere
PREFIX = /usr/local
name = alphaclock

src = $(wildcard src/*.cc)
obj = $(src:.cc=.o)
bin = $(name)

CXXFLAGS = -std=c++11 -pedantic -Wall -g -DPREFIX=\"$(PREFIX)\" -DAPP_NAME=\"$(name)\"
LDFLAGS = -lX11 -lGL -ldrawtext

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: install
install: $(bin)
	mkdir -p $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/share/$(name)
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)
	cp data/* $(DESTDIR)$(PREFIX)/share/$(name)/

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
	rm -f $(DESTDIR)$(PREFIX)/share/$(name)/*
	rmdir $(DESTDIR)$(PREFIX)/share/$(name) || true
