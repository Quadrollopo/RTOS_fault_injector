cmake_minimum_required(VERSION 3.16)
if(UNIX)
	project(freeRTOS C CXX)
	set(CMAKE_C_COMPILER "/usr/bin/gcc")
	set(CMAKE_C_FLAGS "-lstdc++ -ggdb3 -O0 -DprojCOVERAGE_TEST=0 -D_WINDOWS_")
	set(CMAKE_EXE_LINKER_FLAGS "-lstdc++ -ggdb3 -O0 -pthread -lpcap -no-pie")

	add_executable(pds main.cpp Logger.hpp Target.hpp)

	#set(FREERTOS_DIR_REL "../../FreeRTOS")
	set(FREERTOS_DIR_REL "FreeRTOS-cmake/FreeRTOS")
	get_filename_component(FREERTOS_DIR ${FREERTOS_DIR_REL} ABSOLUTE  )

	#set(FREERTOS_PLUS_DIR_REL "../../FreeRTOS-Plus")
	set(FREERTOS_PLUS_DIR_REL "FreeRTOS-cmake/FreeRTOS-Plus")
	get_filename_component(FREERTOS_PLUS_DIR ${FREERTOS_PLUS_DIR_REL} ABSOLUTE  )

	set(BASE "FreeRTOS-cmake/FreeRTOS/Demo")

	#add_executable(freeRTOS "Posix_GCC/main.c" "Posix_GCC/main_andre.c" "Posix_GCC/main_blinky.c" "Posix_GCC/code_coverage_additions.c" "Posix_GCC/console.c"
		#	"Posix_GCC/main_full.c" "Posix_GCC/run-time-stats-utils.c" "Posix_GCC/trcConfig.h" "Posix_GCC/trcSnapshotConfig.h")

	add_executable(freeRTOS "${BASE}/Posix_GCC/main.c" "${BASE}/Posix_GCC/main_andre.c" "${BASE}/Posix_GCC/main_blinky.c" "${BASE}/Posix_GCC/code_coverage_additions.c" "${BASE}/Posix_GCC/console.c"
			"${BASE}/Posix_GCC/main_full.c" "${BASE}/Posix_GCC/run-time-stats-utils.c" "${BASE}/Posix_GCC/trcConfig.h" "${BASE}/Posix_GCC/trcSnapshotConfig.h")

	target_include_directories(freeRTOS PUBLIC "${FREERTOS_DIR}/Source/include"
			"${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix"
			"${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/utils"
			"${FREERTOS_DIR}/Demo/Common/include"
			"${FREERTOS_DIR}/Demo/Posix_GCC"
			"${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/Include")

	#aux_source_directory("." SOURCE)
	aux_source_directory("${FREERTOS_DIR}/Source/" SOURCE)

	target_sources(freeRTOS PUBLIC ${SOURCE}
			${FREERTOS_DIR}/Source/portable/MemMang/heap_3.c
			${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
			${FREERTOS_DIR}/Source/portable/ThirdParty/GCC/Posix/port.c
			${FREERTOS_DIR}/Demo/Common/Minimal/AbortDelay.c
			${FREERTOS_DIR}/Demo/Common/Minimal/BlockQ.c
			${FREERTOS_DIR}/Demo/Common/Minimal/blocktim.c
			${FREERTOS_DIR}/Demo/Common/Minimal/countsem.c
			${FREERTOS_DIR}/Demo/Common/Minimal/death.c
			${FREERTOS_DIR}/Demo/Common/Minimal/dynamic.c
			${FREERTOS_DIR}/Demo/Common/Minimal/EventGroupsDemo.c
			${FREERTOS_DIR}/Demo/Common/Minimal/flop.c
			${FREERTOS_DIR}/Demo/Common/Minimal/GenQTest.c
			${FREERTOS_DIR}/Demo/Common/Minimal/integer.c
			${FREERTOS_DIR}/Demo/Common/Minimal/IntSemTest.c
			${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferAMP.c
			${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferDemo.c
			${FREERTOS_DIR}/Demo/Common/Minimal/PollQ.c
			${FREERTOS_DIR}/Demo/Common/Minimal/QPeek.c
			${FREERTOS_DIR}/Demo/Common/Minimal/QueueOverwrite.c
			${FREERTOS_DIR}/Demo/Common/Minimal/QueueSet.c
			${FREERTOS_DIR}/Demo/Common/Minimal/QueueSetPolling.c
			${FREERTOS_DIR}/Demo/Common/Minimal/recmutex.c
			${FREERTOS_DIR}/Demo/Common/Minimal/semtest.c
			${FREERTOS_DIR}/Demo/Common/Minimal/StaticAllocation.c
			${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferDemo.c
			${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferInterrupt.c
			${FREERTOS_DIR}/Demo/Common/Minimal/TaskNotify.c
			${FREERTOS_DIR}/Demo/Common/Minimal/TimerDemo.c
			${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/trcKernelPort.c
			${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/trcSnapshotRecorder.c
			${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/trcStreamingRecorder.c
			${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/streamports/File/trcStreamingPort.c
	)
endif(UNIX)

if(WIN32)
	cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
	set(CMAKE_FILES_DIRECTORY    "WIN32")

project(WIN32 C)

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x86")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Optimised"
    "Release"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_C_FLAGS "")
    set(CMAKE_C_FLAGS_DEBUG "")
    set(CMAKE_C_FLAGS_OPTIMISED "")
    set(CMAKE_C_FLAGS_RELEASE "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_OPTIMISED "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_OPTIMISED "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_OPTIMISED "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_OPTIMISED "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include("WIN32/CMake/Utils.cmake")

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include("WIN32/CMake/GlobalSettingsInclude.cmake" OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Sub-projects
################################################################################



























set(PROJECT_NAME RTOSDemo)

################################################################################
# Source groups
################################################################################
set(Configuration_Files
    "WIN32/FreeRTOSConfig.h"
    "WIN32/Trace_Recorder_Configuration/trcConfig.h"
)
source_group("Configuration Files" FILES ${Configuration_Files})

set(Demo_App_Source
    "WIN32/main.c"
    "WIN32/Run-time-stats-utils.c"
)
source_group("Demo App Source" FILES ${Demo_App_Source})

set(Demo_App_Source__Blinky_Demo
    "WIN32/main_blinky.c"
)
source_group("Demo App Source\\Blinky_Demo" FILES ${Demo_App_Source__Blinky_Demo})

set(Demo_App_Source__FreeRTOS+Trace_Recorder
    "../../FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcKernelPort.c"
    "../../FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcSnapshotRecorder.c"
)
source_group("Demo App Source\\FreeRTOS+Trace Recorder" FILES ${Demo_App_Source__FreeRTOS+Trace_Recorder})

set(Demo_App_Source__Full_Demo
    "WIN32/main_full.c"
)
source_group("Demo App Source\\Full_Demo" FILES ${Demo_App_Source__Full_Demo})

set(Demo_App_Source__Full_Demo__Common_Demo_Tasks
    "Common/Minimal/AbortDelay.c"
    "Common/Minimal/BlockQ.c"
    "Common/Minimal/blocktim.c"
    "Common/Minimal/countsem.c"
    "Common/Minimal/death.c"
    "Common/Minimal/dynamic.c"
    "Common/Minimal/EventGroupsDemo.c"
    "Common/Minimal/flop.c"
    "Common/Minimal/GenQTest.c"
    "Common/Minimal/integer.c"
    "Common/Minimal/IntSemTest.c"
    "Common/Minimal/MessageBufferAMP.c"
    "Common/Minimal/MessageBufferDemo.c"
    "Common/Minimal/PollQ.c"
    "Common/Minimal/QPeek.c"
    "Common/Minimal/QueueOverwrite.c"
    "Common/Minimal/QueueSet.c"
    "Common/Minimal/QueueSetPolling.c"
    "Common/Minimal/recmutex.c"
    "Common/Minimal/semtest.c"
    "Common/Minimal/StaticAllocation.c"
    "Common/Minimal/StreamBufferDemo.c"
    "Common/Minimal/StreamBufferInterrupt.c"
    "Common/Minimal/TaskNotify.c"
    "Common/Minimal/TaskNotifyArray.c"
    "Common/Minimal/TimerDemo.c"
)
source_group("Demo App Source\\Full_Demo\\Common Demo Tasks" FILES ${Demo_App_Source__Full_Demo__Common_Demo_Tasks})

set(FreeRTOS_Source__Include
    "../Source/include/croutine.h"
    "../Source/include/event_groups.h"
    "../Source/include/FreeRTOS.h"
    "../Source/include/list.h"
    "../Source/include/message_buffer.h"
    "../Source/include/portable.h"
    "../Source/include/projdefs.h"
    "../Source/include/queue.h"
    "../Source/include/semphr.h"
    "../Source/include/stream_buffer.h"
    "../Source/include/task.h"
    "../Source/include/timers.h"
    "../Source/portable/MSVC-MingW/portmacro.h"
)
source_group("FreeRTOS Source\\Include" FILES ${FreeRTOS_Source__Include})

set(FreeRTOS_Source__Source
    "../Source/croutine.c"
    "../Source/event_groups.c"
    "../Source/list.c"
    "../Source/queue.c"
    "../Source/stream_buffer.c"
    "../Source/tasks.c"
    "../Source/timers.c"
)
source_group("FreeRTOS Source\\Source" FILES ${FreeRTOS_Source__Source})

set(FreeRTOS_Source__Source__Portable
    "../Source/portable/MemMang/heap_5.c"
    "../Source/portable/MSVC-MingW/port.c"
)
source_group("FreeRTOS Source\\Source\\Portable" FILES ${FreeRTOS_Source__Source__Portable})

set(ALL_FILES
    ${Configuration_Files}
    ${Demo_App_Source}
    ${Demo_App_Source__Blinky_Demo}
    ${Demo_App_Source__FreeRTOS+Trace_Recorder}
    ${Demo_App_Source__Full_Demo}
    ${Demo_App_Source__Full_Demo__Common_Demo_Tasks}
    ${FreeRTOS_Source__Include}
    ${FreeRTOS_Source__Source}
    ${FreeRTOS_Source__Source__Portable}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Target name
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    TARGET_NAME_DEBUG     "RTOSDemo"
    TARGET_NAME_OPTIMISED "RTOSDemo"
    TARGET_NAME_RELEASE   "RTOSDemo"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG     "${CMAKE_CURRENT_SOURCE_DIR}/WIN32/Debug"
    OUTPUT_DIRECTORY_OPTIMISED "${CMAKE_CURRENT_SOURCE_DIR}/WIN32/Debug"
    OUTPUT_DIRECTORY_RELEASE   "${CMAKE_CURRENT_SOURCE_DIR}/WIN32/Debug"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    PDB_OUTPUT_DIRECTORY           "${CMAKE_CURRENT_SOURCE_DIR}/./Debug"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    MultiThreadedDebug
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../Source/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Source/portable/MSVC-MingW;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/WIN32/Trace_Recorder_Configuration;"
    "${CMAKE_CURRENT_SOURCE_DIR}/WIN32/."
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "WIN32;"
    "_DEBUG;"
    "_CONSOLE;"
    "_WIN32_WINNT=0x0601;"
    "WINVER=0x400;"
    "_CRT_SECURE_NO_WARNINGS;"
    "_MBCS"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Od;
        /RTC1;
        /W4;
        /ZI;
        /wd4574;
        /wd4820;
        /wd4668;
        /wd4255;
        /wd4710;
        /wd4210;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        /NOLOGO;
        /DEBUG;
        /MACHINE:X86;
        /SAFESEH:NO;
        /SUBSYSTEM:CONSOLE;
        /INCREMENTAL
    )
endif()


endif(WIN32)
