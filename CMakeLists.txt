cmake_minimum_required(VERSION 3.10)      # 低于 3.10 的 CMake 不支持自动化 Qt 选项

# ─────────────────────────────────────────
# 1. 指定 vcpkg toolchain  (保持相对路径正确)
# ─────────────────────────────────────────
set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty_install/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")                       # 用 CACHE 可避免二次 configure 时被清掉

# ─────────────────────────────────────────
# 2. 项目名（改成 flowdraw，比默认目录名友好）
# ─────────────────────────────────────────
project(flowdraw LANGUAGES CXX)

# ─────────────────────────────────────────
# 3. 全局 C++ 版本（建议 17）
# ─────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─────────────────────────────────────────
# 4. Windows SDK 版本（可留可删）
# ─────────────────────────────────────────
if (WIN32)
    set(CMAKE_SYSTEM_VERSION "10.0.18362.0")
endif()

# ─────────────────────────────────────────
# 5. 可执行文件统一输出到 build/bin
# ─────────────────────────────────────────
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")

# ─────────────────────────────────────────
# 6. 自动递归所有子目录，但排除隐藏/特殊目录
#    ① 你之后新增源码子目录(如 app, core, gui) 只需建文件夹+子 CMakeLists
#    ② 如果不想自动扫描，可用手写 add_subdirectory
# ─────────────────────────────────────────
set(EXCLUDE_DIRS ".git" "thirdparty_install")
file(GLOB ALL_ITEMS RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/*)

foreach(ITEM ${ALL_ITEMS})
    if (IS_DIRECTORY "${CMAKE_SOURCE_DIR}/${ITEM}")
        list(FIND EXCLUDE_DIRS "${ITEM}" IDX)
        if (IDX EQUAL -1)
            add_subdirectory("${ITEM}")
        endif()
    endif()
endforeach()

# ─────────────────────────────────────────
# 7. VS 启动项目 —— 指向你的主可执行 (改成 app 或 FlowDraw)
# ─────────────────────────────────────────
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY VS_STARTUP_PROJECT app)   # ← 与子目录 project() 名保持一致
