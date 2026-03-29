CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
TARGET   = draw

# Automatically find all .cc files
SRCS = $(wildcard *.cc)
# Turn foo.cc into foo.o (object files)
OBJS = $(SRCS:.cc=.o)
# Turn foo.cc into foo.d (dependency files)
DEPS = $(SRCS:.cc=.d)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile and generate dependency file in one pass
%.o: %.cc
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

# Include all .d files (the - prefix suppresses errors if they don't exist yet)
-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS)

distclean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

.PHONY: clean distclean
