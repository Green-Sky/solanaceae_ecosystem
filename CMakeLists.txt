cmake_minimum_required(VERSION 3.14...3.24 FATAL_ERROR)

# cmake setup begin
project(ecosystem)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# defaulting to debug mode, if not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

# setup my vim ycm :D
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# more paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(SOLANACEAE_ECOSYSTEM_BUILD_TESTING "Build tests" ${BUILD_TESTING})
message("II SOLANACEAE_ECOSYSTEM_BUILD_TESTING " ${SOLANACEAE_ECOSYSTEM_BUILD_TESTING})
option(SOLANACEAE_ECOSYSTEM_ASAN "Build eco with asan (gcc/clang/msvc)" OFF)

# uggly, but it needs to be defined for all dependencies too
# what if its always export?
add_compile_definitions(ENTT_API_EXPORT)

if (SOLANACEAE_ECOSYSTEM_BUILD_TESTING)
	include(CTest)
endif()

if (SOLANACEAE_ECOSYSTEM_ASAN)
	if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
		if (NOT WIN32) # exclude mingw
			add_compile_options(-fno-omit-frame-pointer)
			add_compile_options(-fsanitize=address,undefined)
			#add_compile_options(-fsanitize=address,undefined,pointer-compare,pointer-subtract)
			#add_compile_options(-fhardened)
			#add_compile_options(-D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS -ftrivial-auto-var-init=zero -fPIE  -pie  -Wl,-z,relro,-z,now -fstack-protector-strong -fstack-clash-protection -fcf-protection=full)

			add_link_options(-fno-omit-frame-pointer)
			add_link_options(-fsanitize=address,undefined)
			#add_link_options(-fsanitize=address,undefined,pointer-compare,pointer-subtract)
			#add_link_options(-fhardened)
			#add_link_options(-D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS -ftrivial-auto-var-init=zero -fPIE  -pie  -Wl,-z,relro,-z,now -fstack-protector-strong -fstack-clash-protection -fcf-protection=full)

			link_libraries(-static-libasan) # make it "work" on nix
			message("II enabled ASAN")
		else()
			message("!! can not enable ASAN on this platform (gcc/clang + win)")
		endif()
	elseif (MSVC)
		add_compile_options("/fsanitize=address")
		message("II enabled ASAN")
	else()
		message("!! can not enable ASAN on this platform")
	endif()
endif()

# external libs
add_subdirectory(./external) # before increasing warn levels, sad :(

set(CMAKE_CXX_EXTENSIONS OFF)

# bump up warning levels appropriately for clang, gcc & msvc
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	add_compile_options(
		-Wall -Wextra # Reasonable and standard
		-Wpedantic # Warn if non-standard C++ is used
		-Wunused # Warn on anything being unused
		#-Wconversion # Warn on type conversions that may lose data
		#-Wsign-conversion # Warn on sign conversions
		-Wshadow # Warn if a variable declaration shadows one from a parent context
	)

	add_compile_options(-fno-omit-frame-pointer)
	add_link_options(-fno-omit-frame-pointer)

	if (NOT WIN32)
		#link_libraries(-fsanitize=address,undefined)
		#link_libraries(-fsanitize=undefined)
	endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()

	# remove unreferenced objects, reducing binary and debugsymbol sizes
	# does not work !!! (it actually slightly increased the file size by <1%)
	# add_link_options("/OPT:REF")
endif()

# cmake setup end

add_subdirectory(./plugins)
