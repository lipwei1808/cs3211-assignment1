CC = clang
CXX = clang++

CFLAGS := $(CFLAGS) -g -O3 -Wall -Wextra -pedantic -Werror -std=c18 -pthread
CXXFLAGS := $(CXXFLAGS) -g -O3 -Wall -Wextra -pedantic -Werror -std=c++20 -pthread

BUILDDIR = build
BUILD_TEST_DIR = build/unit_tests

SRCS = main.cpp engine.cpp io.cpp 
TEST_SRCS = atomic_map_test.cpp

all: engine client test

engine: $(SRCS:%=$(BUILDDIR)/%.o)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $(BUILDDIR)/$@

client: $(BUILDDIR)/client.cpp.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $(BUILDDIR)/$@

test: $(BUILD_TEST_DIR) gen-test

gen-test: $(TEST_SRCS:%=$(BUILD_TEST_DIR)/%.o)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $(BUILDDIR)/$@

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
	rm -f client engine gen-test

DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILDDIR)/$<.d
COMPILE.cpp = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

$(BUILDDIR)/%.cpp.o: %.cpp | $(BUILDDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(BUILD_TEST_DIR)/%.cpp.o: unit_tests/%.cpp 
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

$(BUILDDIR): ; @mkdir -p $@

$(BUILD_TEST_DIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%=$(BUILDDIR)/%.d) $(BUILDDIR)/client.cpp.d

-include $(DEPFILES)
