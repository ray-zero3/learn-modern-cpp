# ============================================================
# Chapter 01 reference (お手本)
# ------------------------------------------------------------
# この章で触れる概念:
#   - find_package(OpenCV) で外部ライブラリを使う
#   - RAII クラスのビルド構成
#   - target_link_libraries の PUBLIC vs PRIVATE
# ============================================================

# ------------------------------------------------------------
# 1) ライブラリ: chapter02_ws_camera
# ------------------------------------------------------------
add_library(chapter02_ws_camera
    src/camera_capture.cpp
)

# ヘッダの探索パス: PUBLIC にして、リンク先にも伝搬させる。
# リンクするターゲット (app, tests) が #include "media/..." を
# 解決できるようにするため。
target_include_directories(chapter02_ws_camera PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# C++20 を要件として明示する。
target_compile_features(chapter02_ws_camera PUBLIC cxx_std_20)

# OpenCV をリンクする。
# ${OpenCV_LIBS} はルートの find_package(OpenCV REQUIRED) で設定される。
# PRIVATE: OpenCV はこのライブラリの実装詳細。外部に漏らさない。
# ただし include パスは PUBLIC で伝搬させる必要があるため、
# PUBLIC ${OpenCV_LIBS} にしても問題ないが、PRIVATE が正しい意図。
target_link_libraries(chapter02_ws_camera PUBLIC
    ${OpenCV_LIBS}
)

# OpenCV のヘッダ探索パスも伝搬させる（${OpenCV_INCLUDE_DIRS}）。
# target_link_libraries の IMPORTED target 経由で自動設定される場合もあるが
# 明示的に書くと安全。
target_include_directories(chapter02_ws_camera PUBLIC
    ${OpenCV_INCLUDE_DIRS}
)

modern_cpp_enable_warnings(chapter02_ws_camera)

# ------------------------------------------------------------
# 2) テスト: chapter02_ws_tests
# ------------------------------------------------------------
add_executable(chapter02_ws_tests
    tests/test_camera_capture.cpp
)

target_link_libraries(chapter02_ws_tests
    PRIVATE
        chapter02_ws_camera
        doctest::doctest
)

modern_cpp_enable_warnings(chapter02_ws_tests)

# doctest が各 TEST_CASE を CTest に登録する。
include(doctest)
doctest_discover_tests(chapter02_ws_tests
    TEST_PREFIX "ws::"
)

# ------------------------------------------------------------
# 3) アプリ: chapter02_ws_app
# ------------------------------------------------------------
add_executable(chapter02_ws_app
    src/main.cpp
)

target_link_libraries(chapter02_ws_app PRIVATE
    chapter02_ws_camera
)

modern_cpp_enable_warnings(chapter02_ws_app)
