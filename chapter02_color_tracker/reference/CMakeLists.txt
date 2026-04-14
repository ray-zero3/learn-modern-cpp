# ============================================================
# Chapter 02 reference (お手本)
# ------------------------------------------------------------
# この章で追加するターゲット:
#   chapter02_ref_tracker : HSV トラッカーライブラリ
#   chapter02_ref_tests   : doctest テスト
#   chapter02_ref_app     : カメラ + トラッカーのデモアプリ
# ============================================================

# ------------------------------------------------------------
# 1) CameraCapture ライブラリ (Chapter 01 から継続)
# ------------------------------------------------------------
add_library(chapter02_ref_camera
    src/camera_capture.cpp
)

target_include_directories(chapter02_ref_camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter02_ref_camera PUBLIC cxx_std_20)

target_link_libraries(chapter02_ref_camera PUBLIC
    ${OpenCV_LIBS}
)

target_include_directories(chapter02_ref_camera PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

modern_cpp_enable_warnings(chapter02_ref_camera)

# ------------------------------------------------------------
# 2) ColorTracker ライブラリ
# ------------------------------------------------------------
add_library(chapter02_ref_tracker
    src/color_tracker.cpp
)

target_include_directories(chapter02_ref_tracker PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter02_ref_tracker PUBLIC cxx_std_20)

# OpenCV をリンク。chapter02_ref_camera 経由でも得られるが
# 明示的に指定して依存関係を分かりやすくする。
target_link_libraries(chapter02_ref_tracker PUBLIC
    ${OpenCV_LIBS}
)

target_include_directories(chapter02_ref_tracker PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

modern_cpp_enable_warnings(chapter02_ref_tracker)

# ------------------------------------------------------------
# 3) テスト: chapter02_ref_tests
# ------------------------------------------------------------
add_executable(chapter02_ref_tests
    tests/test_color_tracker.cpp
)

target_link_libraries(chapter02_ref_tests
    PRIVATE
        chapter02_ref_tracker
        doctest::doctest
)

modern_cpp_enable_warnings(chapter02_ref_tests)

include(doctest)
doctest_discover_tests(chapter02_ref_tests)

# ------------------------------------------------------------
# 4) アプリ: chapter02_ref_app
# ------------------------------------------------------------
add_executable(chapter02_ref_app
    app/main.cpp
)

target_link_libraries(chapter02_ref_app PRIVATE
    chapter02_ref_camera
    chapter02_ref_tracker
)

modern_cpp_enable_warnings(chapter02_ref_app)
