# ============================================================
# Chapter 04 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: concept / requires 節 / 静的ディスパッチ
# ============================================================

# ------------------------------------------------------------
# 1) CameraCapture ライブラリ
# ------------------------------------------------------------
add_library(chapter05_ws_camera
    src/camera_capture.cpp
)
target_include_directories(chapter05_ws_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter05_ws_camera PUBLIC cxx_std_20)
target_link_libraries(chapter05_ws_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter05_ws_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter05_ws_camera)

# ------------------------------------------------------------
# 2) Tracker ライブラリ (ColorTracker + Detector concept)
# ------------------------------------------------------------
add_library(chapter05_ws_tracker
    src/color_tracker.cpp
)
target_include_directories(chapter05_ws_tracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter05_ws_tracker PUBLIC cxx_std_20)
target_link_libraries(chapter05_ws_tracker PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter05_ws_tracker PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter05_ws_tracker)

# ------------------------------------------------------------
# 3) OscSender ライブラリ
# ------------------------------------------------------------
add_library(chapter05_ws_osc
    src/osc_sender.cpp
)
target_include_directories(chapter05_ws_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter05_ws_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter05_ws_osc PUBLIC oscpack)
    target_compile_options(chapter05_ws_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter05_ws_osc)

# ------------------------------------------------------------
# 4) テスト: chapter05_ws_tests
# ------------------------------------------------------------
add_executable(chapter05_ws_tests
    tests/test_detector.cpp
)
target_link_libraries(chapter05_ws_tests
    PRIVATE
        chapter05_ws_tracker
        doctest::doctest
)
modern_cpp_enable_warnings(chapter05_ws_tests)

include(doctest)
doctest_discover_tests(chapter05_ws_tests
    TEST_PREFIX "ws::"
)

# ------------------------------------------------------------
# 5) アプリ: chapter05_ws_app
# ------------------------------------------------------------
add_executable(chapter05_ws_app
    app/main.cpp
)
target_link_libraries(chapter05_ws_app PRIVATE
    chapter05_ws_camera
    chapter05_ws_tracker
    chapter05_ws_osc
)
modern_cpp_enable_warnings(chapter05_ws_app)
