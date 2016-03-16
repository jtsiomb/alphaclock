src = $(wildcard src/*.cc)
obj = $(src:.cc=.o)
bin = alphaclock

CXXFLAGS = -std=c++11 -pedantic -Wall -g
LDFLAGS = -lX11 -lGL -ldrawtext

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
