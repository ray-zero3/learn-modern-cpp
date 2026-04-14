# ============================================================
# Chapter 00 - reference (お手本)
# ------------------------------------------------------------
# この章で触れる CMake コマンド:
#   add_library       … 静的/共有ライブラリを定義
#   add_executable    … 実行ファイルを定義
#   target_link_libraries     … 依存リンクを宣言
#   target_include_directories … ヘッダ探索パス
#   target_compile_features    … 機能要件 (cxx_std_20 など)
#
# 「target_*」系を一貫して使うのがモダン CMake の作法。
# ディレクトリ全体に効くグローバル命令（include_directories
# 等）は避ける。
# ============================================================

# ------------------------------------------------------------
# 1) ライブラリ: chapter00_ref_greet
# ------------------------------------------------------------
add_library(chapter00_ref_greet
    src/greeting.cpp
)

# ヘッダの探索パス:
#   PUBLIC にすると、このライブラリをリンクする側にも伝搬する。
#   = 「このライブラリのヘッダを #include したい人はここを見ろ」
target_include_directories(chapter00_ref_greet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 機能要件。ルートでも CMAKE_CXX_STANDARD=20 を指定しているが、
# ターゲット単位でも宣言しておくと、このライブラリを他所から
# add_subdirectory したときに安全。
target_compile_features(chapter00_ref_greet PUBLIC cxx_std_20)

# 警告の有効化 (cmake/Warnings.cmake で定義)
modern_cpp_enable_warnings(chapter00_ref_greet)

# ------------------------------------------------------------
# 2) 実行ファイル: chapter00_ref_app
# ------------------------------------------------------------
add_executable(chapter00_ref_app app/main.cpp)
target_link_libraries(chapter00_ref_app PRIVATE chapter00_ref_greet)
modern_cpp_enable_warnings(chapter00_ref_app)

# ------------------------------------------------------------
# 3) テスト: chapter00_ref_tests
# ------------------------------------------------------------
add_executable(chapter00_ref_tests tests/test_greeting.cpp)
target_link_libraries(chapter00_ref_tests
    PRIVATE
        chapter00_ref_greet
        doctest::doctest
)
modern_cpp_enable_warnings(chapter00_ref_tests)

# doctest 付属の CMake スクリプトが、各 TEST_CASE を CTest の
# 個別テストとして登録してくれる。失敗時に TEST_CASE 名が出る。
include(doctest)
doctest_discover_tests(chapter00_ref_tests)
