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
include CMakeFiles/CXS.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/CXS.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/CXS.dir/flags.make

CMakeFiles/CXS.dir/code/log.cc.o: CMakeFiles/CXS.dir/flags.make
CMakeFiles/CXS.dir/code/log.cc.o: ../code/log.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/CXS.dir/code/log.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CXS.dir/code/log.cc.o -c /cxs/code/log.cc

CMakeFiles/CXS.dir/code/log.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CXS.dir/code/log.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/code/log.cc > CMakeFiles/CXS.dir/code/log.cc.i

CMakeFiles/CXS.dir/code/log.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CXS.dir/code/log.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/code/log.cc -o CMakeFiles/CXS.dir/code/log.cc.s

CMakeFiles/CXS.dir/code/util.cc.o: CMakeFiles/CXS.dir/flags.make
CMakeFiles/CXS.dir/code/util.cc.o: ../code/util.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/CXS.dir/code/util.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CXS.dir/code/util.cc.o -c /cxs/code/util.cc

CMakeFiles/CXS.dir/code/util.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CXS.dir/code/util.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/code/util.cc > CMakeFiles/CXS.dir/code/util.cc.i

CMakeFiles/CXS.dir/code/util.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CXS.dir/code/util.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/code/util.cc -o CMakeFiles/CXS.dir/code/util.cc.s

CMakeFiles/CXS.dir/code/config.cc.o: CMakeFiles/CXS.dir/flags.make
CMakeFiles/CXS.dir/code/config.cc.o: ../code/config.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/CXS.dir/code/config.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CXS.dir/code/config.cc.o -c /cxs/code/config.cc

CMakeFiles/CXS.dir/code/config.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CXS.dir/code/config.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/code/config.cc > CMakeFiles/CXS.dir/code/config.cc.i

CMakeFiles/CXS.dir/code/config.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CXS.dir/code/config.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/code/config.cc -o CMakeFiles/CXS.dir/code/config.cc.s

CMakeFiles/CXS.dir/code/thread.cc.o: CMakeFiles/CXS.dir/flags.make
CMakeFiles/CXS.dir/code/thread.cc.o: ../code/thread.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/CXS.dir/code/thread.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CXS.dir/code/thread.cc.o -c /cxs/code/thread.cc

CMakeFiles/CXS.dir/code/thread.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CXS.dir/code/thread.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/code/thread.cc > CMakeFiles/CXS.dir/code/thread.cc.i

CMakeFiles/CXS.dir/code/thread.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CXS.dir/code/thread.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/code/thread.cc -o CMakeFiles/CXS.dir/code/thread.cc.s

CMakeFiles/CXS.dir/code/fiber.cc.o: CMakeFiles/CXS.dir/flags.make
CMakeFiles/CXS.dir/code/fiber.cc.o: ../code/fiber.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/CXS.dir/code/fiber.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CXS.dir/code/fiber.cc.o -c /cxs/code/fiber.cc

CMakeFiles/CXS.dir/code/fiber.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CXS.dir/code/fiber.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/code/fiber.cc > CMakeFiles/CXS.dir/code/fiber.cc.i

CMakeFiles/CXS.dir/code/fiber.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CXS.dir/code/fiber.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/code/fiber.cc -o CMakeFiles/CXS.dir/code/fiber.cc.s

CMakeFiles/CXS.dir/code/scheduler.cc.o: CMakeFiles/CXS.dir/flags.make
CMakeFiles/CXS.dir/code/scheduler.cc.o: ../code/scheduler.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/CXS.dir/code/scheduler.cc.o"
	/opt/rh/devtoolset-9/root/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CXS.dir/code/scheduler.cc.o -c /cxs/code/scheduler.cc

CMakeFiles/CXS.dir/code/scheduler.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CXS.dir/code/scheduler.cc.i"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /cxs/code/scheduler.cc > CMakeFiles/CXS.dir/code/scheduler.cc.i

CMakeFiles/CXS.dir/code/scheduler.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CXS.dir/code/scheduler.cc.s"
	/opt/rh/devtoolset-9/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /cxs/code/scheduler.cc -o CMakeFiles/CXS.dir/code/scheduler.cc.s

# Object files for target CXS
CXS_OBJECTS = \
"CMakeFiles/CXS.dir/code/log.cc.o" \
"CMakeFiles/CXS.dir/code/util.cc.o" \
"CMakeFiles/CXS.dir/code/config.cc.o" \
"CMakeFiles/CXS.dir/code/thread.cc.o" \
"CMakeFiles/CXS.dir/code/fiber.cc.o" \
"CMakeFiles/CXS.dir/code/scheduler.cc.o"

# External object files for target CXS
CXS_EXTERNAL_OBJECTS =

libCXS.so: CMakeFiles/CXS.dir/code/log.cc.o
libCXS.so: CMakeFiles/CXS.dir/code/util.cc.o
libCXS.so: CMakeFiles/CXS.dir/code/config.cc.o
libCXS.so: CMakeFiles/CXS.dir/code/thread.cc.o
libCXS.so: CMakeFiles/CXS.dir/code/fiber.cc.o
libCXS.so: CMakeFiles/CXS.dir/code/scheduler.cc.o
libCXS.so: CMakeFiles/CXS.dir/build.make
libCXS.so: CMakeFiles/CXS.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/cxs/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX shared library libCXS.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/CXS.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/CXS.dir/build: libCXS.so

.PHONY : CMakeFiles/CXS.dir/build

CMakeFiles/CXS.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/CXS.dir/cmake_clean.cmake
.PHONY : CMakeFiles/CXS.dir/clean

CMakeFiles/CXS.dir/depend:
	cd /cxs/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /cxs /cxs /cxs/build /cxs/build /cxs/build/CMakeFiles/CXS.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/CXS.dir/depend

