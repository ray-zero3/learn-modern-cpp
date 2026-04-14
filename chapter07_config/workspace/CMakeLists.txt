# ============================================================
# Chapter 06 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: std::jthread, std::stop_token, SPSC キュー
# ============================================================

# カメラライブラリ
add_library(chapter07_ws_camera src/camera_capture.cpp)
target_include_directories(chapter07_ws_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ws_camera PUBLIC cxx_std_20)
target_link_libraries(chapter07_ws_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter07_ws_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter07_ws_camera)

# Detector ライブラリ
add_library(chapter07_ws_detector
    src/color_tracker.cpp
    src/optical_flow.cpp
)
target_include_directories(chapter07_ws_detector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ws_detector PUBLIC cxx_std_20)
target_link_libraries(chapter07_ws_detector PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter07_ws_detector PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter07_ws_detector)

# OSC ライブラリ
add_library(chapter07_ws_osc src/osc_sender.cpp)
target_include_directories(chapter07_ws_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ws_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter07_ws_osc PUBLIC oscpack)
    target_compile_options(chapter07_ws_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter07_ws_osc)

# パイプラインライブラリ
add_library(chapter07_ws_pipeline src/pipeline.cpp)
target_include_directories(chapter07_ws_pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ws_pipeline PUBLIC cxx_std_20)
target_link_libraries(chapter07_ws_pipeline PUBLIC
    chapter07_ws_camera
    chapter07_ws_detector
    chapter07_ws_osc
)
modern_cpp_enable_warnings(chapter07_ws_pipeline)

# スレッドライブラリを探して追加（macOS は libc++ に含まれるが明示すると安全）
find_package(Threads REQUIRED)
target_link_libraries(chapter07_ws_pipeline PUBLIC Threads::Threads)

# テスト（SPSC キュー）
add_executable(chapter07_ws_tests tests/test_pipeline.cpp)
target_link_libraries(chapter07_ws_tests PRIVATE
    chapter07_ws_pipeline
    doctest::doctest
    Threads::Threads
)
modern_cpp_enable_warnings(chapter07_ws_tests)

include(doctest)
doctest_discover_tests(chapter07_ws_tests
    TEST_PREFIX "ws::"
)

# アプリ
add_executable(chapter07_ws_app src/main.cpp)
target_link_libraries(chapter07_ws_app PRIVATE
    chapter07_ws_pipeline
)
modern_cpp_enable_warnings(chapter07_ws_app)
