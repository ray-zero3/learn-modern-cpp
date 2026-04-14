# ============================================================
# Chapter 05 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: 新 Detector の追加、std::array vs std::vector
# ============================================================

# カメラライブラリ
add_library(chapter06_ws_camera src/camera_capture.cpp)
target_include_directories(chapter06_ws_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter06_ws_camera PUBLIC cxx_std_20)
target_link_libraries(chapter06_ws_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter06_ws_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter06_ws_camera)

# トラッカー + フロー ライブラリ
add_library(chapter06_ws_detector
    src/color_tracker.cpp
    src/optical_flow.cpp
)
target_include_directories(chapter06_ws_detector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter06_ws_detector PUBLIC cxx_std_20)
target_link_libraries(chapter06_ws_detector PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter06_ws_detector PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter06_ws_detector)

# OscSender ライブラリ
add_library(chapter06_ws_osc src/osc_sender.cpp)
target_include_directories(chapter06_ws_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter06_ws_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter06_ws_osc PUBLIC oscpack)
    target_compile_options(chapter06_ws_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter06_ws_osc)

# テスト
add_executable(chapter06_ws_tests tests/test_optical_flow.cpp)
target_link_libraries(chapter06_ws_tests PRIVATE
    chapter06_ws_detector
    doctest::doctest
)
modern_cpp_enable_warnings(chapter06_ws_tests)

include(doctest)
doctest_discover_tests(chapter06_ws_tests
    TEST_PREFIX "ws::"
)

# アプリ
add_executable(chapter06_ws_app app/main.cpp)
target_link_libraries(chapter06_ws_app PRIVATE
    chapter06_ws_camera
    chapter06_ws_detector
    chapter06_ws_osc
)
modern_cpp_enable_warnings(chapter06_ws_app)
