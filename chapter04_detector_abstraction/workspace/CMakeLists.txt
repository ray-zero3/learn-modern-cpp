# ============================================================
# Chapter 03 reference (お手本)
# ------------------------------------------------------------
# この章で追加するもの:
#   - oscpack 依存の OSC 送信ライブラリ
#   - std::variant を使った OscValue 型
# ============================================================

# ------------------------------------------------------------
# 1) CameraCapture ライブラリ (Chapter 01 から継続)
# ------------------------------------------------------------
add_library(chapter04_ws_camera
    src/camera_capture.cpp
)

target_include_directories(chapter04_ws_camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter04_ws_camera PUBLIC cxx_std_20)
target_link_libraries(chapter04_ws_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter04_ws_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter04_ws_camera)

# ------------------------------------------------------------
# 2) ColorTracker ライブラリ (Chapter 02 から継続)
# ------------------------------------------------------------
add_library(chapter04_ws_tracker
    src/color_tracker.cpp
)

target_include_directories(chapter04_ws_tracker PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter04_ws_tracker PUBLIC cxx_std_20)
target_link_libraries(chapter04_ws_tracker PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter04_ws_tracker PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter04_ws_tracker)

# ------------------------------------------------------------
# 3) OscSender ライブラリ (この章で新規追加)
# ------------------------------------------------------------
add_library(chapter04_ws_osc
    src/osc_sender.cpp
)

target_include_directories(chapter04_ws_osc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter04_ws_osc PUBLIC cxx_std_20)

# oscpack は arm64 で型定義が壊れるため、自前実装をデフォルトとする。
# MODERN_CPP_USE_OSCPACK=ON の場合のみ oscpack にリンク。
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter04_ws_osc PUBLIC oscpack)
    target_compile_options(chapter04_ws_osc PRIVATE -Wno-deprecated-declarations)
    target_compile_definitions(chapter04_ws_osc PRIVATE MODERN_CPP_WITH_OSCPACK)
endif()

modern_cpp_enable_warnings(chapter04_ws_osc)

# ------------------------------------------------------------
# 4) テスト: chapter04_ws_tests
# ------------------------------------------------------------
add_executable(chapter04_ws_tests
    tests/test_osc_sender.cpp
)

target_link_libraries(chapter04_ws_tests
    PRIVATE
        chapter04_ws_osc
        doctest::doctest
)

modern_cpp_enable_warnings(chapter04_ws_tests)

include(doctest)
doctest_discover_tests(chapter04_ws_tests
    TEST_PREFIX "ws::"
)

# ------------------------------------------------------------
# 5) アプリ: chapter04_ws_app
# ------------------------------------------------------------
add_executable(chapter04_ws_app
    src/main.cpp
)

target_link_libraries(chapter04_ws_app PRIVATE
    chapter04_ws_camera
    chapter04_ws_tracker
    chapter04_ws_osc
)

modern_cpp_enable_warnings(chapter04_ws_app)
