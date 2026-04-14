# ============================================================
# Chapter 07 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: std::expected, toml++, 設定ファイル対応
# ============================================================

# カメラライブラリ
add_library(chapter07_ref_camera src/camera_capture.cpp)
target_include_directories(chapter07_ref_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ref_camera PUBLIC cxx_std_20)
target_link_libraries(chapter07_ref_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter07_ref_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter07_ref_camera)

# Detector ライブラリ
add_library(chapter07_ref_detector
    src/color_tracker.cpp
    src/optical_flow.cpp
)
target_include_directories(chapter07_ref_detector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ref_detector PUBLIC cxx_std_20)
target_link_libraries(chapter07_ref_detector PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter07_ref_detector PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter07_ref_detector)

# OSC ライブラリ
add_library(chapter07_ref_osc src/osc_sender.cpp)
target_include_directories(chapter07_ref_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ref_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter07_ref_osc PUBLIC oscpack)
    target_compile_options(chapter07_ref_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter07_ref_osc)

# パイプラインライブラリ
find_package(Threads REQUIRED)
add_library(chapter07_ref_pipeline src/pipeline.cpp)
target_include_directories(chapter07_ref_pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ref_pipeline PUBLIC cxx_std_20)
target_link_libraries(chapter07_ref_pipeline PUBLIC
    chapter07_ref_camera
    chapter07_ref_detector
    chapter07_ref_osc
    Threads::Threads
)
modern_cpp_enable_warnings(chapter07_ref_pipeline)

# Config ライブラリ (この章で新規追加)
add_library(chapter07_ref_config src/config.cpp)
target_include_directories(chapter07_ref_config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter07_ref_config PUBLIC cxx_std_23)  # std::expected は C++23
target_link_libraries(chapter07_ref_config PUBLIC
    chapter07_ref_detector  # HsvRange を使う
    tomlplusplus::tomlplusplus  # toml++ の CMake ターゲット
)
modern_cpp_enable_warnings(chapter07_ref_config)
# toml++ は一部の警告が出るため、このターゲットだけ緩める
target_compile_options(chapter07_ref_config PRIVATE -Wno-old-style-cast)

# テスト
add_executable(chapter07_ref_tests tests/test_config.cpp)
target_link_libraries(chapter07_ref_tests PRIVATE
    chapter07_ref_config
    doctest::doctest
)
modern_cpp_enable_warnings(chapter07_ref_tests)
# テストでも old-style-cast 警告を緩める（toml++ 経由）
target_compile_options(chapter07_ref_tests PRIVATE -Wno-old-style-cast)

include(doctest)
doctest_discover_tests(chapter07_ref_tests)

# アプリ
add_executable(chapter07_ref_app app/main.cpp)
target_link_libraries(chapter07_ref_app PRIVATE
    chapter07_ref_camera
    chapter07_ref_detector
    chapter07_ref_osc
    chapter07_ref_config
)
modern_cpp_enable_warnings(chapter07_ref_app)
