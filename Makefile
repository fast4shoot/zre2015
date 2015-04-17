CXXFLAGS += -g -std=c++11

PROGRAM = zre_proj1_linux
SOURCES = main.cpp filter.cpp
HEADERS = filter.h

HEADERDIR = src
SRCDIR = src
OBJDIR = obj

OBJSFULL = $(addprefix $(OBJDIR)/,$(addsuffix .o, $(SOURCES)))
SOURCESFULL = $(addprefix $(SRCDIR)/,$(SOURCES))
HEADERSFULL = $(addprefix $(HEADERDIR)/,$(HEADERS))

all: $(PROGRAM)

clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(PROGRAM)

$(PROGRAM): $(OBJSFULL)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $? 

$(OBJDIR)/%.o: $(SRCDIR)/% $(HEADERSFULL)
	mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

.PHONY: all

