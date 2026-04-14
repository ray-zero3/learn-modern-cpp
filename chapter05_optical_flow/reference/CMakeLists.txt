# ============================================================
# Chapter 05 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: 新 Detector の追加、std::array vs std::vector
# ============================================================

# カメラライブラリ
add_library(chapter05_ref_camera src/camera_capture.cpp)
target_include_directories(chapter05_ref_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter05_ref_camera PUBLIC cxx_std_20)
target_link_libraries(chapter05_ref_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter05_ref_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter05_ref_camera)

# トラッカー + フロー ライブラリ
add_library(chapter05_ref_detector
    src/color_tracker.cpp
    src/optical_flow.cpp
)
target_include_directories(chapter05_ref_detector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter05_ref_detector PUBLIC cxx_std_20)
target_link_libraries(chapter05_ref_detector PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter05_ref_detector PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter05_ref_detector)

# OscSender ライブラリ
add_library(chapter05_ref_osc src/osc_sender.cpp)
target_include_directories(chapter05_ref_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter05_ref_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter05_ref_osc PUBLIC oscpack)
    target_compile_options(chapter05_ref_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter05_ref_osc)

# テスト
add_executable(chapter05_ref_tests tests/test_optical_flow.cpp)
target_link_libraries(chapter05_ref_tests PRIVATE
    chapter05_ref_detector
    doctest::doctest
)
modern_cpp_enable_warnings(chapter05_ref_tests)

include(doctest)
doctest_discover_tests(chapter05_ref_tests)

# アプリ
add_executable(chapter05_ref_app src/main.cpp)
target_link_libraries(chapter05_ref_app PRIVATE
    chapter05_ref_camera
    chapter05_ref_detector
    chapter05_ref_osc
)
modern_cpp_enable_warnings(chapter05_ref_app)
