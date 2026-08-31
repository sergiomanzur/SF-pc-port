set(PSYCROSS_SRCS_C
	src/render/glad.c
	src/util/timer.c
	src/psx/ABS.C
	src/psx/INLINE_C.C
	src/psx/LIBAPI.C
	src/psx/LIBCD.C
	src/psx/LIBETC.C
	src/psx/LIBGPU.C
	src/psx/LIBGTE.C
	src/psx/LIBMCRD.C
	src/psx/LIBPAD.C
	src/psx/LIBSN.C
	src/psx/LIBSPU.C
)

set(PSYCROSS_SRCS_CPP
	src/audio/PsyX_SPUAL.cpp
	src/gpu/PsyX_GPU.cpp
	src/gte/half_float.cpp
	src/gte/PsyX_GTE.cpp
	src/pad/PsyX_pad.cpp
	src/render/PsyX_render.cpp
	src/util/crash_handler.cpp
	src/PsyX_main.cpp
)

set_source_files_properties(${PSYCROSS_SRCS_C} PROPERTIES LANGUAGE C)
set_source_files_properties(${PSYCROSS_SRCS_CPP} PROPERTIES LANGUAGE CXX)
#set_source_files_properties( "src/gte/PsyX_GTE.cpp" PROPERTIES LANGUAGE C)

add_library(psycross_static STATIC ${PSYCROSS_SRCS_C} ${PSYCROSS_SRCS_CPP})
set_target_properties(psycross_static PROPERTIES OUTPUT_NAME "psycross")
target_include_directories(psycross_static PUBLIC "include" "include/psx")

if(NOT TARGET SDL2 AND NOT TARGET SDL2::SDL2 AND NOT TARGET SDL2-static)
	find_package(SDL2 REQUIRED)
endif()
if(TARGET SDL2-static)
	target_link_libraries(psycross_static SDL2-static)
elseif(TARGET SDL2::SDL2)
	target_link_libraries(psycross_static SDL2::SDL2)
elseif(TARGET SDL2)
	target_link_libraries(psycross_static SDL2)
else()
	target_link_libraries(psycross_static ${SDL2_LIBRARIES})
	target_include_directories(psycross_static PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

if(NOT TARGET OpenAL AND NOT TARGET OpenAL::OpenAL)
	find_package(OPENAL REQUIRED)
endif()
target_link_libraries(psycross_static OpenAL)
