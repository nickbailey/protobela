# Makefile for all programs in the bela sandpit
# with automatic dependency scanning

BELASTUB := bela.a
bela_SRCS := render.cxx

#PROGRAM := hsl

SRCS := $(bela_SRCS)

OBJS := $(SRCS:.cxx=.o)
DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
DEPFILES := $(SRCS:%.cxx=$(DEPDIR)/%.d)

.PHONY: all clean
all: $(BELASTUB) #$(PROGRAM)

clean:
	$(RM) $(OBJS) $(PROGRAM) -r $(DEPDIR)

$(BELASTUB): $(bela_SRCS:.cxx=.o)
	$(AR) rc $@ $^

#$(PROGRAM): $(OBJS)
#	$(LINK.cc) $(OUTPUT_OPTION) $^

%.o: %.cxx $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.cc) $<

$(DEPDIR):
	mkdir -p $@

$(DEPFILES):

include  $(wildcard $(DEPFILES))

