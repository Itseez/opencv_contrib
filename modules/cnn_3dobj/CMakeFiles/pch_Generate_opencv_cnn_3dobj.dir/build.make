# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/cmake-gui

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wangyida/Downloads/opencv-3.0.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wangyida/Desktop/opencv_contrib

# Utility rule file for pch_Generate_opencv_cnn_3dobj.

# Include the progress variables for this target.
include modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/progress.make

modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj: modules/cnn_3dobj/precomp.hpp.gch/opencv_cnn_3dobj_Release.gch

modules/cnn_3dobj/precomp.hpp.gch/opencv_cnn_3dobj_Release.gch: modules/cnn_3dobj/src/precomp.hpp
modules/cnn_3dobj/precomp.hpp.gch/opencv_cnn_3dobj_Release.gch: modules/cnn_3dobj/precomp.hpp
modules/cnn_3dobj/precomp.hpp.gch/opencv_cnn_3dobj_Release.gch: lib/libopencv_cnn_3dobj_pch_dephelp.a
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wangyida/Desktop/opencv_contrib/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating precomp.hpp.gch/opencv_cnn_3dobj_Release.gch"
	cd /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj && /usr/bin/cmake -E make_directory /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/precomp.hpp.gch
	cd /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj && /usr/bin/c++ -O3 -DNDEBUG -DNDEBUG -fPIC -DCVAPI_EXPORTS -isystem"/home/wangyida/Downloads/opencv-3.0.0/3rdparty/ippicv/unpack/ippicv_lnx/include" -isystem"/home/wangyida/Desktop/opencv_contrib" -isystem"/home/wangyida/Downloads/opencv-3.0.0/3rdparty/ippicv/unpack/ippicv_lnx/include" -isystem"/home/wangyida/Desktop/opencv_contrib" -isystem"/home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/include" -isystem"/home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/src" -isystem"/home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj" -I"/home/wangyida/Downloads/opencv-3.0.0/modules/hal/include" -I"/home/wangyida/Downloads/opencv-3.0.0/modules/core/include" -I"/home/wangyida/Downloads/opencv-3.0.0/modules/imgproc/include" -I"/home/wangyida/Downloads/opencv-3.0.0/modules/viz/include" -I"/home/wangyida/Downloads/opencv-3.0.0/modules/imgcodecs/include" -I"/home/wangyida/Downloads/opencv-3.0.0/modules/videoio/include" -I"/home/wangyida/Downloads/opencv-3.0.0/modules/highgui/include" -D__OPENCV_BUILD=1 -fsigned-char -W -Wall -Werror=return-type -Werror=non-virtual-dtor -Werror=address -Werror=sequence-point -Wformat -Werror=format-security -Wmissing-declarations -Wundef -Winit-self -Wpointer-arith -Wshadow -Wsign-promo -Wno-narrowing -Wno-delete-non-virtual-dtor -fdiagnostics-show-option -Wno-long-long -pthread -fomit-frame-pointer -msse -msse2 -mno-avx -msse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -ffunction-sections -fvisibility=hidden -fvisibility-inlines-hidden -DCVAPI_EXPORTS -x c++-header -o /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/precomp.hpp.gch/opencv_cnn_3dobj_Release.gch /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/precomp.hpp

modules/cnn_3dobj/precomp.hpp: modules/cnn_3dobj/src/precomp.hpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wangyida/Desktop/opencv_contrib/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating precomp.hpp"
	cd /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj && /usr/bin/cmake -E copy /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/src/precomp.hpp /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/precomp.hpp

pch_Generate_opencv_cnn_3dobj: modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj
pch_Generate_opencv_cnn_3dobj: modules/cnn_3dobj/precomp.hpp.gch/opencv_cnn_3dobj_Release.gch
pch_Generate_opencv_cnn_3dobj: modules/cnn_3dobj/precomp.hpp
pch_Generate_opencv_cnn_3dobj: modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/build.make
.PHONY : pch_Generate_opencv_cnn_3dobj

# Rule to build all files generated by this target.
modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/build: pch_Generate_opencv_cnn_3dobj
.PHONY : modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/build

modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/clean:
	cd /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj && $(CMAKE_COMMAND) -P CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/cmake_clean.cmake
.PHONY : modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/clean

modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/depend:
	cd /home/wangyida/Desktop/opencv_contrib && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wangyida/Downloads/opencv-3.0.0 /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj /home/wangyida/Desktop/opencv_contrib /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj /home/wangyida/Desktop/opencv_contrib/modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : modules/cnn_3dobj/CMakeFiles/pch_Generate_opencv_cnn_3dobj.dir/depend

