# ============================================================
# Chapter 06 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: std::jthread, std::stop_token, SPSC キュー
# ============================================================

# カメラライブラリ
add_library(chapter06_ref_camera src/camera_capture.cpp)
target_include_directories(chapter06_ref_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter06_ref_camera PUBLIC cxx_std_20)
target_link_libraries(chapter06_ref_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter06_ref_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter06_ref_camera)

# Detector ライブラリ
add_library(chapter06_ref_detector
    src/color_tracker.cpp
    src/optical_flow.cpp
)
target_include_directories(chapter06_ref_detector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter06_ref_detector PUBLIC cxx_std_20)
target_link_libraries(chapter06_ref_detector PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter06_ref_detector PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter06_ref_detector)

# OSC ライブラリ
add_library(chapter06_ref_osc src/osc_sender.cpp)
target_include_directories(chapter06_ref_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter06_ref_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter06_ref_osc PUBLIC oscpack)
    target_compile_options(chapter06_ref_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter06_ref_osc)

# パイプラインライブラリ
add_library(chapter06_ref_pipeline src/pipeline.cpp)
target_include_directories(chapter06_ref_pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter06_ref_pipeline PUBLIC cxx_std_20)
target_link_libraries(chapter06_ref_pipeline PUBLIC
    chapter06_ref_camera
    chapter06_ref_detector
    chapter06_ref_osc
)
modern_cpp_enable_warnings(chapter06_ref_pipeline)

# スレッドライブラリを探して追加（macOS は libc++ に含まれるが明示すると安全）
find_package(Threads REQUIRED)
target_link_libraries(chapter06_ref_pipeline PUBLIC Threads::Threads)

# テスト（SPSC キュー）
add_executable(chapter06_ref_tests tests/test_pipeline.cpp)
target_link_libraries(chapter06_ref_tests PRIVATE
    chapter06_ref_pipeline
    doctest::doctest
    Threads::Threads
)
modern_cpp_enable_warnings(chapter06_ref_tests)

include(doctest)
doctest_discover_tests(chapter06_ref_tests)

# アプリ
add_executable(chapter06_ref_app src/main.cpp)
target_link_libraries(chapter06_ref_app PRIVATE
    chapter06_ref_pipeline
)
modern_cpp_enable_warnings(chapter06_ref_app)
