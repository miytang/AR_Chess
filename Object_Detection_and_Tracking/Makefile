TARGETS=Detect_Markers
SOURCES=Detect_Markers.cpp
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCES))
DEPENDS=$(patsubst %.cpp,%.d,$(SOURCES))

CC = g++

CPPFLAGS=
CXXFLAGS=`pkg-config --cflags opencv glu glew`
LDFLAGS=`pkg-config --libs-only-L opencv glu glew`
LDLIBS=`pkg-config --libs-only-l opencv glu glew` -lglut
.PHONY: all clean

all: $(TARGETS)
$(TARGETS): $(OBJECTS)

clean:
	-$(RM) $(TARGETS) $(OBJECTS) $(DEPENDS) *~

%.d: %.cpp
	@set -e; $(CC) -MM $(CXXFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; [ -s $@ ] || rm -f $@
-include $(DEPENDS)
