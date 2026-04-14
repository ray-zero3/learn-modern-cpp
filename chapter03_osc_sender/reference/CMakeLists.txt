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
add_library(chapter03_ref_camera
    src/camera_capture.cpp
)

target_include_directories(chapter03_ref_camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter03_ref_camera PUBLIC cxx_std_20)
target_link_libraries(chapter03_ref_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter03_ref_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter03_ref_camera)

# ------------------------------------------------------------
# 2) ColorTracker ライブラリ (Chapter 02 から継続)
# ------------------------------------------------------------
add_library(chapter03_ref_tracker
    src/color_tracker.cpp
)

target_include_directories(chapter03_ref_tracker PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter03_ref_tracker PUBLIC cxx_std_20)
target_link_libraries(chapter03_ref_tracker PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter03_ref_tracker PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter03_ref_tracker)

# ------------------------------------------------------------
# 3) OscSender ライブラリ (この章で新規追加)
# ------------------------------------------------------------
add_library(chapter03_ref_osc
    src/osc_sender.cpp
)

target_include_directories(chapter03_ref_osc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(chapter03_ref_osc PUBLIC cxx_std_20)

# oscpack は arm64 で型定義が壊れるため、自前実装をデフォルトとする。
# MODERN_CPP_USE_OSCPACK=ON の場合のみ oscpack にリンク。
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter03_ref_osc PUBLIC oscpack)
    target_compile_options(chapter03_ref_osc PRIVATE -Wno-deprecated-declarations)
    target_compile_definitions(chapter03_ref_osc PRIVATE MODERN_CPP_WITH_OSCPACK)
endif()

modern_cpp_enable_warnings(chapter03_ref_osc)

# ------------------------------------------------------------
# 4) テスト: chapter03_ref_tests
# ------------------------------------------------------------
add_executable(chapter03_ref_tests
    tests/test_osc_sender.cpp
)

target_link_libraries(chapter03_ref_tests
    PRIVATE
        chapter03_ref_osc
        doctest::doctest
)

modern_cpp_enable_warnings(chapter03_ref_tests)

include(doctest)
doctest_discover_tests(chapter03_ref_tests)

# ------------------------------------------------------------
# 5) アプリ: chapter03_ref_app
# ------------------------------------------------------------
add_executable(chapter03_ref_app
    app/main.cpp
)

target_link_libraries(chapter03_ref_app PRIVATE
    chapter03_ref_camera
    chapter03_ref_tracker
    chapter03_ref_osc
)

modern_cpp_enable_warnings(chapter03_ref_app)
