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
include CMakeFiles/test_schedule.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/test_schedule.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/test_schedule.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_schedule.dir/flags.make

CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o: CMakeFiles/test_schedule.dir/flags.make
CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o: /home/ecs-user/CXS_Server/CXS_Server/test/test_schedule.cpp
CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o: CMakeFiles/test_schedule.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ecs-user/CXS_Server/CXS_Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o -MF CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o.d -o CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o -c /home/ecs-user/CXS_Server/CXS_Server/test/test_schedule.cpp

CMakeFiles/test_schedule.dir/test/test_schedule.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/test_schedule.dir/test/test_schedule.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ecs-user/CXS_Server/CXS_Server/test/test_schedule.cpp > CMakeFiles/test_schedule.dir/test/test_schedule.cpp.i

CMakeFiles/test_schedule.dir/test/test_schedule.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/test_schedule.dir/test/test_schedule.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ecs-user/CXS_Server/CXS_Server/test/test_schedule.cpp -o CMakeFiles/test_schedule.dir/test/test_schedule.cpp.s

# Object files for target test_schedule
test_schedule_OBJECTS = \
"CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o"

# External object files for target test_schedule
test_schedule_EXTERNAL_OBJECTS =

/home/ecs-user/CXS_Server/CXS_Server/bin/test_schedule: CMakeFiles/test_schedule.dir/test/test_schedule.cpp.o
/home/ecs-user/CXS_Server/CXS_Server/bin/test_schedule: CMakeFiles/test_schedule.dir/build.make
/home/ecs-user/CXS_Server/CXS_Server/bin/test_schedule: libCXS.so
/home/ecs-user/CXS_Server/CXS_Server/bin/test_schedule: CMakeFiles/test_schedule.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/ecs-user/CXS_Server/CXS_Server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable /home/ecs-user/CXS_Server/CXS_Server/bin/test_schedule"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_schedule.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_schedule.dir/build: /home/ecs-user/CXS_Server/CXS_Server/bin/test_schedule
.PHONY : CMakeFiles/test_schedule.dir/build

CMakeFiles/test_schedule.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_schedule.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_schedule.dir/clean

CMakeFiles/test_schedule.dir/depend:
	cd /home/ecs-user/CXS_Server/CXS_Server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ecs-user/CXS_Server/CXS_Server /home/ecs-user/CXS_Server/CXS_Server /home/ecs-user/CXS_Server/CXS_Server/build /home/ecs-user/CXS_Server/CXS_Server/build /home/ecs-user/CXS_Server/CXS_Server/build/CMakeFiles/test_schedule.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/test_schedule.dir/depend

