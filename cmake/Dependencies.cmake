# ============================================================
# Dependencies.cmake
# ------------------------------------------------------------
# 外部依存を FetchContent で取り込む。
# TS でいう package.json の dependencies に相当。
#
# FetchContent_Declare で「どこから何を取るか」を宣言し、
# FetchContent_MakeAvailable で実体をダウンロード&取り込み。
# ============================================================

include(FetchContent)

# ------------------------------------------------------------
# doctest - 軽量ヘッダオンリーテストフレームワーク
# 選定理由: ヘッダ1枚、CMake 統合が容易、コンパイル時間が速い
#           Catch2 より起動オーバーヘッドが小さい
# ------------------------------------------------------------
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
    GIT_SHALLOW    TRUE
)

# doctest の内蔵テスト/例を含めない（ビルド時間短縮）
set(DOCTEST_WITH_TESTS OFF CACHE INTERNAL "")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE INTERNAL "")
set(DOCTEST_NO_INSTALL ON CACHE INTERNAL "")

# doctest v2.4.11 の CMakeLists は cmake_minimum_required が古く、
# CMake 4.x で拒否される。FetchContent のサブプロジェクトだけに
# 古いポリシー互換を許可する。
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

FetchContent_MakeAvailable(doctest)

# doctest には CMake スクリプト `doctest_discover_tests` が付属。
# CMAKE_MODULE_PATH に scripts/cmake を足すと include できる。
list(APPEND CMAKE_MODULE_PATH "${doctest_SOURCE_DIR}/scripts/cmake")
set(MODERN_CPP_DOCTEST_CMAKE_DIR "${doctest_SOURCE_DIR}/scripts/cmake"
    CACHE INTERNAL "doctest の CMake スクリプト置き場")

# ------------------------------------------------------------
# oscpack - OSC (Open Sound Control) の送受信ライブラリ
# 選定理由: C++ ネイティブ、軽量、OSC 1.0 準拠
# 代替案:
#   - liblo: C ライブラリ（Homebrew で取れるが CMake 統合が面倒）
#   - 自前実装: oscpack が取得できない場合のフォールバック
#
# 注意: oscpack の CMakeLists.txt は存在しないリポジトリもある。
# GIT_TAG master でプロジェクト側の CMake ファイルを使う。
# ビルドできない場合は MODERN_CPP_USE_OSCPACK=OFF にして
# 章の自前実装（minimal_osc.hpp）にフォールバックする。
# ------------------------------------------------------------
# oscpack は arm64 (Apple Silicon) で int32 の型定義が不正になる問題がある
# (long が 8 バイトなのに int32 として使う)。
# そのため、自前の最小 OSC 実装をデフォルトとする。
option(MODERN_CPP_USE_OSCPACK "oscpack ライブラリを使う（OFF=自前実装）" OFF)

if(MODERN_CPP_USE_OSCPACK)
    FetchContent_Declare(
        oscpack
        GIT_REPOSITORY https://github.com/RossBencina/oscpack.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )

    # oscpack は CMakeLists.txt を持たないため、
    # FetchContent_MakeAvailable ではなく手動で INTERFACE ライブラリを作る。
    FetchContent_GetProperties(oscpack)
    if(NOT oscpack_POPULATED)
        FetchContent_Populate(oscpack)
    endif()

    # oscpack のヘッダとソースを集めて STATIC ライブラリとして定義する。
    # oscpack の構造: osc/ ip/ (POSIX or Win32) に分かれている
    if(UNIX OR APPLE)
        add_library(oscpack STATIC
            ${oscpack_SOURCE_DIR}/osc/OscTypes.cpp
            ${oscpack_SOURCE_DIR}/osc/OscReceivedElements.cpp
            ${oscpack_SOURCE_DIR}/osc/OscPrintReceivedElements.cpp
            ${oscpack_SOURCE_DIR}/osc/OscOutboundPacketStream.cpp
            ${oscpack_SOURCE_DIR}/ip/IpEndpointName.cpp
            ${oscpack_SOURCE_DIR}/ip/posix/NetworkingUtils.cpp
            ${oscpack_SOURCE_DIR}/ip/posix/UdpSocket.cpp
        )
    else()
        # Windows 向け (参考: このプロジェクトは macOS 前提)
        add_library(oscpack STATIC
            ${oscpack_SOURCE_DIR}/osc/OscTypes.cpp
            ${oscpack_SOURCE_DIR}/osc/OscReceivedElements.cpp
            ${oscpack_SOURCE_DIR}/osc/OscPrintReceivedElements.cpp
            ${oscpack_SOURCE_DIR}/osc/OscOutboundPacketStream.cpp
            ${oscpack_SOURCE_DIR}/ip/IpEndpointName.cpp
            ${oscpack_SOURCE_DIR}/ip/win32/NetworkingUtils.cpp
            ${oscpack_SOURCE_DIR}/ip/win32/UdpSocket.cpp
        )
    endif()

    target_include_directories(oscpack PUBLIC
        ${oscpack_SOURCE_DIR}
    )

    # oscpack はスレッドを使用
    find_package(Threads REQUIRED)
    target_link_libraries(oscpack PUBLIC Threads::Threads)

    set(MODERN_CPP_OSCPACK_AVAILABLE ON CACHE INTERNAL "oscpack が利用可能")
else()
    set(MODERN_CPP_OSCPACK_AVAILABLE OFF CACHE INTERNAL "oscpack が利用不可（自前実装使用）")
endif()

# ------------------------------------------------------------
# toml++ - TOML 設定ファイルパーサー (Chapter 07 で使用)
# 選定理由: ヘッダオンリー、C++17 対応、型安全な API
# ------------------------------------------------------------
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(tomlplusplus)

# ------------------------------------------------------------
# Dear ImGui - 即席 GUI フレームワーク (Chapter 09 で使用)
# 選定理由: ヘッダベース、CMake 統合が容易、軽量
# backends: GLFW + OpenGL3 を使用
# ------------------------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.9
    GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# ImGui 本体（コアソース）
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
# ImGui の古いスタイルキャスト警告を抑制
target_compile_options(imgui PRIVATE -Wno-old-style-cast -Wno-cast-align)

# ImGui GLFW + OpenGL3 バックエンド
find_package(glfw3 QUIET)
find_package(OpenGL QUIET)

if(glfw3_FOUND AND OpenGL_FOUND)
    add_library(imgui_backends STATIC
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui_backends PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_backends PUBLIC
        imgui
        glfw
        OpenGL::GL
    )
    target_compile_options(imgui_backends PRIVATE
        -Wno-old-style-cast -Wno-cast-align -Wno-conversion)
    set(MODERN_CPP_IMGUI_AVAILABLE ON CACHE INTERNAL "ImGui が利用可能")
    message(STATUS "Dear ImGui: GLFW + OpenGL3 バックエンドが利用可能")
else()
    set(MODERN_CPP_IMGUI_AVAILABLE OFF CACHE INTERNAL "ImGui は利用不可")
    message(STATUS "Dear ImGui: glfw3 または OpenGL が見つかりません。GUI 機能を無効化します。")
    if(NOT glfw3_FOUND)
        message(STATUS "  brew install glfw でインストールできます")
    endif()
endif()
