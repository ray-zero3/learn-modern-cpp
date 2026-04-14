# ============================================================
# Chapter 04 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: concept / requires 節 / 静的ディスパッチ
# ============================================================

# ------------------------------------------------------------
# 1) CameraCapture ライブラリ
# ------------------------------------------------------------
add_library(chapter04_ref_camera
    src/camera_capture.cpp
)
target_include_directories(chapter04_ref_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter04_ref_camera PUBLIC cxx_std_20)
target_link_libraries(chapter04_ref_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter04_ref_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter04_ref_camera)

# ------------------------------------------------------------
# 2) Tracker ライブラリ (ColorTracker + Detector concept)
# ------------------------------------------------------------
add_library(chapter04_ref_tracker
    src/color_tracker.cpp
)
target_include_directories(chapter04_ref_tracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter04_ref_tracker PUBLIC cxx_std_20)
target_link_libraries(chapter04_ref_tracker PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter04_ref_tracker PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter04_ref_tracker)

# ------------------------------------------------------------
# 3) OscSender ライブラリ
# ------------------------------------------------------------
add_library(chapter04_ref_osc
    src/osc_sender.cpp
)
target_include_directories(chapter04_ref_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter04_ref_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter04_ref_osc PUBLIC oscpack)
    target_compile_options(chapter04_ref_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter04_ref_osc)

# ------------------------------------------------------------
# 4) テスト: chapter04_ref_tests
# ------------------------------------------------------------
add_executable(chapter04_ref_tests
    tests/test_detector.cpp
)
target_link_libraries(chapter04_ref_tests
    PRIVATE
        chapter04_ref_tracker
        doctest::doctest
)
modern_cpp_enable_warnings(chapter04_ref_tests)

include(doctest)
doctest_discover_tests(chapter04_ref_tests)

# ------------------------------------------------------------
# 5) アプリ: chapter04_ref_app
# ------------------------------------------------------------
add_executable(chapter04_ref_app
    app/main.cpp
)
target_link_libraries(chapter04_ref_app PRIVATE
    chapter04_ref_camera
    chapter04_ref_tracker
    chapter04_ref_osc
)
modern_cpp_enable_warnings(chapter04_ref_app)
