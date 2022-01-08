cmake_minimum_required(VERSION 3.16)
if(UNIX)
	project(freeRTOS C CXX)
	set(CMAKE_C_COMPILER "/usr/bin/gcc")
	set(CMAKE_C_FLAGS "-lstdc++ -ggdb3 -O0 -DprojCOVERAGE_TEST=0 ")
	set(CMAKE_EXE_LINKER_FLAGS "-lstdc++ -ggdb3 -O0 -pthread -no-pie")

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

	add_executable(freeRTOS "${BASE}/Posix_GCC/main.c" "${BASE}/Posix_GCC/main_blinky.c" "${BASE}/Posix_GCC/code_coverage_additions.c" "${BASE}/Posix_GCC/console.c"
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
	set(BASE "FreeRTOS-cmake/FreeRTOS/Demo")

################################################################################
# Sub-projects
################################################################################
project(RTOSDemo C CXX)

add_executable(pds main_win.cpp Logger.hpp Target.hpp)

################################################################################
# Source groups
################################################################################
set(Configuration_Files
    "${BASE}/WIN32/FreeRTOSConfig.h"
    "${BASE}/WIN32/Trace_Recorder_Configuration/trcConfig.h"
)

set(Demo_App_Source
    "${BASE}/WIN32/main.c"
    "${BASE}/WIN32/Run-time-stats-utils.c"
)

set(Demo_App_Source__Blinky_Demo
    "${BASE}/WIN32/main_blinky.c"
)

set(Demo_App_Source__FreeRTOS+Trace_Recorder
    "${BASE}/../../FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcKernelPort.c"
    "${BASE}/../../FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcSnapshotRecorder.c"
)

set(Demo_App_Source__Full_Demo
    "${BASE}/WIN32/main_full.c"
)

set(Demo_App_Source__Full_Demo__Common_Demo_Tasks
    "${BASE}/Common/Minimal/AbortDelay.c"
    "${BASE}/Common/Minimal/BlockQ.c"
    "${BASE}/Common/Minimal/blocktim.c"
    "${BASE}/Common/Minimal/countsem.c"
    "${BASE}/Common/Minimal/death.c"
    "${BASE}/Common/Minimal/dynamic.c"
    "${BASE}/Common/Minimal/EventGroupsDemo.c"
    "${BASE}/Common/Minimal/flop.c"
    "${BASE}/Common/Minimal/GenQTest.c"
    "${BASE}/Common/Minimal/integer.c"
    "${BASE}/Common/Minimal/IntSemTest.c"
    "${BASE}/Common/Minimal/MessageBufferAMP.c"
    "${BASE}/Common/Minimal/MessageBufferDemo.c"
    "${BASE}/Common/Minimal/PollQ.c"
    "${BASE}/Common/Minimal/QPeek.c"
    "${BASE}/Common/Minimal/QueueOverwrite.c"
    "${BASE}/Common/Minimal/QueueSet.c"
    "${BASE}/Common/Minimal/QueueSetPolling.c"
    "${BASE}/Common/Minimal/recmutex.c"
    "${BASE}/Common/Minimal/semtest.c"
    "${BASE}/Common/Minimal/StaticAllocation.c"
    "${BASE}/Common/Minimal/StreamBufferDemo.c"
    "${BASE}/Common/Minimal/StreamBufferInterrupt.c"
    "${BASE}/Common/Minimal/TaskNotify.c"
    "${BASE}/Common/Minimal/TaskNotifyArray.c"
    "${BASE}/Common/Minimal/TimerDemo.c"
)

set(FreeRTOS_Source__Include
    "${BASE}/../Source/include/croutine.h"
    "${BASE}/../Source/include/event_groups.h"
    "${BASE}/../Source/include/FreeRTOS.h"
    "${BASE}/../Source/include/list.h"
    "${BASE}/../Source/include/message_buffer.h"
    "${BASE}/../Source/include/portable.h"
    "${BASE}/../Source/include/projdefs.h"
    "${BASE}/../Source/include/queue.h"
    "${BASE}/../Source/include/semphr.h"
    "${BASE}/../Source/include/stream_buffer.h"
    "${BASE}/../Source/include/task.h"
    "${BASE}/../Source/include/timers.h"
    "${BASE}/../Source/portable/MSVC-MingW/portmacro.h"
)

set(FreeRTOS_Source__Source
    "${BASE}/../Source/croutine.c"
    "${BASE}/../Source/event_groups.c"
    "${BASE}/../Source/list.c"
    "${BASE}/../Source/queue.c"
    "${BASE}/../Source/stream_buffer.c"
    "${BASE}/../Source/tasks.c"
    "${BASE}/../Source/timers.c"
)

set(FreeRTOS_Source__Source__Portable
    "${BASE}/../Source/portable/MemMang/heap_5.c"
    "${BASE}/../Source/portable/MSVC-MingW/port.c"
)

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
	if(MSVC)
		target_link_options(RTOSDemo PUBLIC /DYNAMICBASE:NO /BASE:0x400000 /INCREMENTAL:NO /OPT:NOREF)
	endif()
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

################################################################################
# Include directories
################################################################################
	set(CMAKE_CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-cmake/FreeRTOS/Demo")
target_include_directories(${PROJECT_NAME} PUBLIC

    "${CMAKE_CURRENT_SOURCE_DIR}/../Source/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Source/portable/MSVC-MingW;"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/WIN32/Trace_Recorder_Configuration;"
    "${CMAKE_CURRENT_SOURCE_DIR}/WIN32/."
)


endif(WIN32)
