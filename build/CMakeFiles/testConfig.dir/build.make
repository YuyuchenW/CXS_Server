# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/cmake/bin/cmake

# The command to remove a file.
RM = /usr/local/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /cxs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /cxs/build

# Include any dependencies generated for this target.
include CMakeFiles/testConfig.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/testConfig.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testConfig.dir/flags.make

CMakeFiles/testConfig.dir/test/test_config.cc.o: CMakeFiles/testConfig.dir/flags.make
CMakeFiles/testConfig.dir/test/test_config.cc.o: ../test/test_config.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testConfig.dir/test/test_config.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testConfig.dir/test/test_config.cc.o -c /cxs/test/test_config.cc

CMakeFiles/testConfig.dir/test/test_config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testConfig.dir/test/test_config.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/test/test_config.cc > CMakeFiles/testConfig.dir/test/test_config.cc.i

CMakeFiles/testConfig.dir/test/test_config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testConfig.dir/test/test_config.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/test/test_config.cc -o CMakeFiles/testConfig.dir/test/test_config.cc.s

# Object files for target testConfig
testConfig_OBJECTS = \
"CMakeFiles/testConfig.dir/test/test_config.cc.o"

# External object files for target testConfig
testConfig_EXTERNAL_OBJECTS =

../bin/testConfig: CMakeFiles/testConfig.dir/test/test_config.cc.o
../bin/testConfig: CMakeFiles/testConfig.dir/build.make
../bin/testConfig: libCXS.so
../bin/testConfig: CMakeFiles/testConfig.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/testConfig"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testConfig.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testConfig.dir/build: ../bin/testConfig

.PHONY : CMakeFiles/testConfig.dir/build

CMakeFiles/testConfig.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testConfig.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testConfig.dir/clean

CMakeFiles/testConfig.dir/depend:
	cd /cxs/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /cxs /cxs /cxs/build /cxs/build /cxs/build/CMakeFiles/testConfig.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/testConfig.dir/depend

