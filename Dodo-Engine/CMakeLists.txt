cmake_minimum_required(VERSION 3.12.2)

project(Dodo-Engine)

foreach (EnvVar IN ITEMS VULKAN_SDK)
  if ("$ENV{${EnvVar}}" STREQUAL "")
    message(WARNING, "Environmental variable '${EnvVar}' is not defined.")
  endif ()
endforeach ()
set(CMAKE_CONFIGURATION_TYPES "Debug"
    CACHE STRING "Configuration types" FORCE)

add_executable(Dodo-Engine
  code/common/DodoTypes.h
  code/common/VKHelpers.h
  code/common/VKIntegration.cpp
  code/Common/VKIntegration.h
  code/components/ECS.h
  code/components/Material.h
  code/components/Transform.h
  code/dodopch.cpp
  code/dodopch.h
  code/engine/Engine.cpp
  code/engine/Engine.h
  code/entity/Entity.cpp
  code/entity/Entity.h
  code/entity/EntityHandler.cpp
  code/entity/EntityHandler.h
  code/environment/Error.cpp
  code/environment/Error.h
  code/environment/Log.cpp
  code/environment/Log.h
  code/environment/Window.cpp
  code/environment/Window.h
  code/main.cpp
  code/renderer/Renderer.cpp
  code/renderer/Renderer.h
  ext/spdlog/async.h
  ext/spdlog/async_logger.h
  ext/spdlog/common.h
  ext/spdlog/details/async_logger_impl.h
  ext/spdlog/details/circular_q.h
  ext/spdlog/details/console_globals.h
  ext/spdlog/details/file_helper.h
  ext/spdlog/details/fmt_helper.h
  ext/spdlog/details/log_msg.h
  ext/spdlog/details/logger_impl.h
  ext/spdlog/details/mpmc_blocking_q.h
  ext/spdlog/details/null_mutex.h
  ext/spdlog/details/os.h
  ext/spdlog/details/pattern_formatter.h
  ext/spdlog/details/periodic_worker.h
  ext/spdlog/details/registry.h
  ext/spdlog/details/thread_pool.h
  ext/spdlog/fmt/bin_to_hex.h
  ext/spdlog/fmt/bundled/chrono.h
  ext/spdlog/fmt/bundled/color.h
  ext/spdlog/fmt/bundled/core.h
  ext/spdlog/fmt/bundled/format.h
  ext/spdlog/fmt/bundled/format-inl.h
  ext/spdlog/fmt/bundled/locale.h
  ext/spdlog/fmt/bundled/ostream.h
  ext/spdlog/fmt/bundled/posix.h
  ext/spdlog/fmt/bundled/printf.h
  ext/spdlog/fmt/bundled/ranges.h
  ext/spdlog/fmt/bundled/time.h
  ext/spdlog/fmt/fmt.h
  ext/spdlog/fmt/ostr.h
  ext/spdlog/formatter.h
  ext/spdlog/logger.h
  ext/spdlog/sinks/android_sink.h
  ext/spdlog/sinks/ansicolor_sink.h
  ext/spdlog/sinks/base_sink.h
  ext/spdlog/sinks/basic_file_sink.h
  ext/spdlog/sinks/daily_file_sink.h
  ext/spdlog/sinks/dist_sink.h
  ext/spdlog/sinks/msvc_sink.h
  ext/spdlog/sinks/null_sink.h
  ext/spdlog/sinks/ostream_sink.h
  ext/spdlog/sinks/rotating_file_sink.h
  ext/spdlog/sinks/sink.h
  ext/spdlog/sinks/stdout_color_sinks.h
  ext/spdlog/sinks/stdout_sinks.h
  ext/spdlog/sinks/syslog_sink.h
  ext/spdlog/sinks/wincolor_sink.h
  ext/spdlog/spdlog.h
  ext/spdlog/tweakme.h
  ext/spdlog/version.h
)
# Additional include directories
set_property(TARGET Dodo-Engine
  APPEND PROPERTY INCLUDE_DIRECTORIES
  $<$<CONFIG:Debug>:
    ${CMAKE_CURRENT_SOURCE_DIR}/code>
)

# Preprocessor definitions
target_compile_definitions(Dodo-Engine PRIVATE
  
)

# SDL check
target_compile_options(Dodo-Engine PRIVATE
  "$<$<CONFIG:Debug>:/sdl>"
)

# Minimal rebuild
if (MSVC)
  target_compile_options(Dodo-Engine PRIVATE
    "$<$<CONFIG:Debug>:/Gm->"
  )
endif ()

# Precompiled header files
if (MSVC)
  target_compile_options(Dodo-Engine PRIVATE
    "$<$<CONFIG:Debug>:/Yu"dodopch.h">"
  )
  set_property(SOURCE code/dodopch.cpp
    APPEND_STRING PROPERTY COMPILE_FLAGS
    "$<$<CONFIG:Debug>:/Yc"dodopch.h">")
endif ()

# Additional library directories
if (MSVC)
  target_link_options(Dodo-Engine PRIVATE
    $<$<CONFIG:Debug>:
      /LIBPATH:D:\Development\Vulkan\glfw-3.2.1.bin.WIN64\lib-vc2015
      /LIBPATH:$ENV{VULKAN_SDK}\Lib>
  )
else ()
  target_link_options(Dodo-Engine PRIVATE
    $<$<CONFIG:Debug>:
      -LD:\Development\Vulkan\glfw-3.2.1.bin.WIN64\lib-vc2015
      -L$ENV{VULKAN_SDK}\Lib>
  )
endif ()

# Link libraries
set_property(TARGET Dodo-Engine
  APPEND PROPERTY LINK_LIBRARIES
  "$<$<CONFIG:Debug>:glfw3.lib;vulkan-1.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib>"
)
