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
include CMakeFiles/testThread.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/testThread.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testThread.dir/flags.make

CMakeFiles/testThread.dir/test/test_thread.cc.o: CMakeFiles/testThread.dir/flags.make
CMakeFiles/testThread.dir/test/test_thread.cc.o: ../test/test_thread.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testThread.dir/test/test_thread.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testThread.dir/test/test_thread.cc.o -c /cxs/test/test_thread.cc

CMakeFiles/testThread.dir/test/test_thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testThread.dir/test/test_thread.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/test/test_thread.cc > CMakeFiles/testThread.dir/test/test_thread.cc.i

CMakeFiles/testThread.dir/test/test_thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testThread.dir/test/test_thread.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/test/test_thread.cc -o CMakeFiles/testThread.dir/test/test_thread.cc.s

# Object files for target testThread
testThread_OBJECTS = \
"CMakeFiles/testThread.dir/test/test_thread.cc.o"

# External object files for target testThread
testThread_EXTERNAL_OBJECTS =

../bin/testThread: CMakeFiles/testThread.dir/test/test_thread.cc.o
../bin/testThread: CMakeFiles/testThread.dir/build.make
../bin/testThread: libCXS.so
../bin/testThread: CMakeFiles/testThread.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/testThread"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testThread.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testThread.dir/build: ../bin/testThread

.PHONY : CMakeFiles/testThread.dir/build

CMakeFiles/testThread.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testThread.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testThread.dir/clean

CMakeFiles/testThread.dir/depend:
	cd /cxs/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /cxs /cxs /cxs/build /cxs/build /cxs/build/CMakeFiles/testThread.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/testThread.dir/depend

