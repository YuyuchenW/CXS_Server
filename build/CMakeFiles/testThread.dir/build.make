# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Produce verbose output by default.
VERBOSE = 1

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ecs-user/CXS_Server/CXS_Server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ecs-user/CXS_Server/CXS_Server/build

# Include any dependencies generated for this target.
include CMakeFiles/testThread.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/testThread.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/testThread.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testThread.dir/flags.make

CMakeFiles/testThread.dir/test/test_thread.cc.o: CMakeFiles/testThread.dir/flags.make
CMakeFiles/testThread.dir/test/test_thread.cc.o: /home/ecs-user/CXS_Server/CXS_Server/test/test_thread.cc
CMakeFiles/testThread.dir/test/test_thread.cc.o: CMakeFiles/testThread.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ecs-user/CXS_Server/CXS_Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testThread.dir/test/test_thread.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testThread.dir/test/test_thread.cc.o -MF CMakeFiles/testThread.dir/test/test_thread.cc.o.d -o CMakeFiles/testThread.dir/test/test_thread.cc.o -c /home/ecs-user/CXS_Server/CXS_Server/test/test_thread.cc

CMakeFiles/testThread.dir/test/test_thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/testThread.dir/test/test_thread.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ecs-user/CXS_Server/CXS_Server/test/test_thread.cc > CMakeFiles/testThread.dir/test/test_thread.cc.i

CMakeFiles/testThread.dir/test/test_thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/testThread.dir/test/test_thread.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ecs-user/CXS_Server/CXS_Server/test/test_thread.cc -o CMakeFiles/testThread.dir/test/test_thread.cc.s

# Object files for target testThread
testThread_OBJECTS = \
"CMakeFiles/testThread.dir/test/test_thread.cc.o"

# External object files for target testThread
testThread_EXTERNAL_OBJECTS =

/home/ecs-user/CXS_Server/CXS_Server/bin/testThread: CMakeFiles/testThread.dir/test/test_thread.cc.o
/home/ecs-user/CXS_Server/CXS_Server/bin/testThread: CMakeFiles/testThread.dir/build.make
/home/ecs-user/CXS_Server/CXS_Server/bin/testThread: libCXS.so
/home/ecs-user/CXS_Server/CXS_Server/bin/testThread: CMakeFiles/testThread.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/ecs-user/CXS_Server/CXS_Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/ecs-user/CXS_Server/CXS_Server/bin/testThread"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testThread.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testThread.dir/build: /home/ecs-user/CXS_Server/CXS_Server/bin/testThread
.PHONY : CMakeFiles/testThread.dir/build

CMakeFiles/testThread.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testThread.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testThread.dir/clean

CMakeFiles/testThread.dir/depend:
	cd /home/ecs-user/CXS_Server/CXS_Server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ecs-user/CXS_Server/CXS_Server /home/ecs-user/CXS_Server/CXS_Server /home/ecs-user/CXS_Server/CXS_Server/build /home/ecs-user/CXS_Server/CXS_Server/build /home/ecs-user/CXS_Server/CXS_Server/build/CMakeFiles/testThread.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/testThread.dir/depend

