# ============================================================
# Chapter 02 reference (お手本)
# ------------------------------------------------------------
# この章で追加するターゲット:
#   chapter03_ws_tracker : HSV トラッカーライブラリ
#   chapter03_ws_tests   : doctest テスト
#   chapter03_ws_app     : カメラ + トラッカーのデモアプリ
# ============================================================

# ------------------------------------------------------------
# 1) CameraCapture ライブラリ (Chapter 01 から継続)
# ------------------------------------------------------------
add_library(chapter03_ws_camera
    src/camera_capture.cpp
)

target_include_directories(chapter03_ws_camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter03_ws_camera PUBLIC cxx_std_20)

target_link_libraries(chapter03_ws_camera PUBLIC
    ${OpenCV_LIBS}
)

target_include_directories(chapter03_ws_camera PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

modern_cpp_enable_warnings(chapter03_ws_camera)

# ------------------------------------------------------------
# 2) ColorTracker ライブラリ
# ------------------------------------------------------------
add_library(chapter03_ws_tracker
    src/color_tracker.cpp
)

target_include_directories(chapter03_ws_tracker PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter03_ws_tracker PUBLIC cxx_std_20)

# OpenCV をリンク。chapter03_ws_camera 経由でも得られるが
# 明示的に指定して依存関係を分かりやすくする。
target_link_libraries(chapter03_ws_tracker PUBLIC
    ${OpenCV_LIBS}
)

target_include_directories(chapter03_ws_tracker PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

modern_cpp_enable_warnings(chapter03_ws_tracker)

# ------------------------------------------------------------
# 3) テスト: chapter03_ws_tests
# ------------------------------------------------------------
add_executable(chapter03_ws_tests
    tests/test_color_tracker.cpp
)

target_link_libraries(chapter03_ws_tests
    PRIVATE
        chapter03_ws_tracker
        doctest::doctest
)

modern_cpp_enable_warnings(chapter03_ws_tests)

include(doctest)
doctest_discover_tests(chapter03_ws_tests
    TEST_PREFIX "ws::"
)

# ------------------------------------------------------------
# 4) アプリ: chapter03_ws_app
# ------------------------------------------------------------
add_executable(chapter03_ws_app
    src/main.cpp
)

target_link_libraries(chapter03_ws_app PRIVATE
    chapter03_ws_camera
    chapter03_ws_tracker
)

modern_cpp_enable_warnings(chapter03_ws_app)
