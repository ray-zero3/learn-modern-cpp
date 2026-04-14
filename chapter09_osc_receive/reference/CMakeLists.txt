# ============================================================
# Chapter 09 reference (お手本)
# ------------------------------------------------------------
# この章で学ぶ: Dear ImGui, OSC 受信, Observer パターン
# ============================================================

find_package(Threads REQUIRED)

# カメラライブラリ
add_library(chapter09_ref_camera src/camera_capture.cpp)
target_include_directories(chapter09_ref_camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter09_ref_camera PUBLIC cxx_std_20)
target_link_libraries(chapter09_ref_camera PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter09_ref_camera PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter09_ref_camera)

# Detector ライブラリ
add_library(chapter09_ref_detector
    src/color_tracker.cpp
    src/optical_flow.cpp
)
target_include_directories(chapter09_ref_detector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter09_ref_detector PUBLIC cxx_std_20)
target_link_libraries(chapter09_ref_detector PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter09_ref_detector PUBLIC ${OpenCV_INCLUDE_DIRS})
modern_cpp_enable_warnings(chapter09_ref_detector)

# OSC ライブラリ
add_library(chapter09_ref_osc src/osc_sender.cpp)
target_include_directories(chapter09_ref_osc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter09_ref_osc PUBLIC cxx_std_20)
if(MODERN_CPP_OSCPACK_AVAILABLE)
    target_link_libraries(chapter09_ref_osc PUBLIC oscpack)
    target_compile_options(chapter09_ref_osc PRIVATE -Wno-deprecated-declarations)
endif()
modern_cpp_enable_warnings(chapter09_ref_osc)

# パイプラインライブラリ
add_library(chapter09_ref_pipeline src/pipeline.cpp)
target_include_directories(chapter09_ref_pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter09_ref_pipeline PUBLIC cxx_std_20)
target_link_libraries(chapter09_ref_pipeline PUBLIC
    chapter09_ref_camera
    chapter09_ref_detector
    chapter09_ref_osc
    Threads::Threads
)
modern_cpp_enable_warnings(chapter09_ref_pipeline)

# Config ライブラリ
add_library(chapter09_ref_config src/config.cpp)
target_include_directories(chapter09_ref_config PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter09_ref_config PUBLIC cxx_std_23)
target_link_libraries(chapter09_ref_config PUBLIC
    chapter09_ref_detector
    tomlplusplus::tomlplusplus
)
target_compile_options(chapter09_ref_config PRIVATE -Wno-old-style-cast)
modern_cpp_enable_warnings(chapter09_ref_config)

# OSC 受信 + UI State ライブラリ (この章で新規追加)
add_library(chapter09_ref_full
    src/osc_receiver.cpp
    src/ui_state.cpp
)
target_include_directories(chapter09_ref_full PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(chapter09_ref_full PUBLIC cxx_std_20)
target_link_libraries(chapter09_ref_full PUBLIC
    chapter09_ref_detector
    chapter09_ref_osc
    Threads::Threads
)
modern_cpp_enable_warnings(chapter09_ref_full)

# テスト (UiState のテストは GUI なしで動く)
add_executable(chapter09_ref_tests tests/test_ui_state.cpp)
target_link_libraries(chapter09_ref_tests PRIVATE
    chapter09_ref_full
    doctest::doctest
)
modern_cpp_enable_warnings(chapter09_ref_tests)

include(doctest)
doctest_discover_tests(chapter09_ref_tests)

# アプリ
add_executable(chapter09_ref_app app/main.cpp)
target_link_libraries(chapter09_ref_app PRIVATE
    chapter09_ref_camera
    chapter09_ref_detector
    chapter09_ref_osc
    chapter09_ref_config
    chapter09_ref_full
)

# ImGui が使える場合はリンクして定義を追加
if(MODERN_CPP_IMGUI_AVAILABLE)
    target_link_libraries(chapter09_ref_app PRIVATE imgui_backends)
    target_compile_definitions(chapter09_ref_app PRIVATE MODERN_CPP_WITH_IMGUI)
    message(STATUS "Chapter 09: ImGui 有効")
else()
    message(STATUS "Chapter 09: ImGui 無効（コンソールモード）")
endif()

modern_cpp_enable_warnings(chapter09_ref_app)
# ImGui のヘッダで発生する警告を抑制
target_compile_options(chapter09_ref_app PRIVATE -Wno-old-style-cast -Wno-cast-align)
