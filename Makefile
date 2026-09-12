# Makefile for all programs in the bela sandpit
# with automatic dependency scanning

PROGRAMS := beep sampleplayer

sampleplayer_SRCS := sampleplayer.cxx samplereader.cxx
beep_SRCS := beep.cxx

BELASTUB := bela.a
bela_SRCS := bellacontext.cxx belaiohelpers.cxx digitalinputsim.cxx

SRCS := $(bela_SRCS) $(beep_SRCS) $(sampleplayer_SRCS)

OBJS := $(SRCS:.cxx=.o)
DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
DEPFILES := $(SRCS:%.cxx=$(DEPDIR)/%.d)
CXXFLAGS += $(shell pkg-config --cflags rtaudio) -std=c++17
LDLIBS += $(shell pkg-config --libs rtaudio)
CXXFLAGS += $(shell pkg-config --cflags sndfile)
LDLIBS += $(shell pkg-config --libs sndfile)

.PHONY: all clean
all: $(BELASTUB) $(PROGRAMS)

clean:
	$(RM) $(OBJS) $(PROGRAMS) $(BELASTUB) -r $(DEPDIR)

$(BELASTUB): $(bela_SRCS:.cxx=.o)
	$(AR) rc $@ $^

beep: $(beep_SRCS:.cxx=.o) $(BELASTUB)
	$(LINK.cc) $(OUTPUT_OPTION) $^ $(LDLIBS)

sampleplayer: $(sampleplayer_SRCS:.cxx=.o) $(BELASTUB)
	$(LINK.cc) $(OUTPUT_OPTION) $^ $(LDLIBS)

%.o: %.cxx $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.cc) $<

$(DEPDIR):
	mkdir -p $@

$(DEPFILES):

include  $(wildcard $(DEPFILES))

