# ============================================================
# modern-cpp : ルート CMakeLists
# ------------------------------------------------------------
# このプロジェクトは「章ごとに独立したサブプロジェクト」を
# add_subdirectory でぶら下げていく構成。
#
# TS に例えるなら、ここは monorepo のルート package.json 兼
# tsconfig のベース設定に相当する。各章は workspace package。
# ============================================================

cmake_minimum_required(VERSION 3.25)

project(modern_cpp
    VERSION 0.1.0
    DESCRIPTION "CV→OSC メディアアートツールで学ぶモダン C++"
    LANGUAGES CXX)

# ------------------------------------------------------------
# C++ 言語規格
# ------------------------------------------------------------
# C++20 を「要求」する。CMAKE_CXX_STANDARD_REQUIRED = ON にすると
# フォールバック（例: C++17 にダウングレード）を禁止する。
# CMAKE_CXX_EXTENSIONS = OFF にすると GNU 拡張を使わず
# ポータブルな範囲に抑える（= std=c++20, not gnu++20）。
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compile_commands.json を生成（clangd / clang-tidy が食う）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# スタンドアロンでビルドされた時だけテストを有効化
# （将来別プロジェクトから add_subdirectory されても壊れないように）
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
endif()

# ------------------------------------------------------------
# 章ごとの切り替えオプション
# ------------------------------------------------------------
# お手本（reference）と作業用（workspace）は個別にビルドを
# 無効化できる。workspace は未実装状態でもリンクは通るように
# スケルトンを用意してあるが、「お手本だけ試したい」ときは
# -DMODERN_CPP_BUILD_WORKSPACE=OFF を使う。
# ------------------------------------------------------------
option(MODERN_CPP_BUILD_REFERENCE "各章の reference (お手本) をビルドする" ON)
option(MODERN_CPP_BUILD_WORKSPACE "各章の workspace (作業用) をビルドする" ON)

# ------------------------------------------------------------
# 共通設定モジュール
# ------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Warnings)        # warning フラグを付ける関数 modern_cpp_enable_warnings
include(Dependencies)    # doctest など外部依存を FetchContent

# ------------------------------------------------------------
# 外部ライブラリ (システム依存)
# ------------------------------------------------------------
# OpenCV: Homebrew 版 (`brew install opencv`) を想定。
# find_package でシステムにインストール済みのものを探す。
# FetchContent と違い、バイナリ済みのものを使う（ビルド時間短縮）。
find_package(OpenCV REQUIRED)

# ------------------------------------------------------------
# 章
# ------------------------------------------------------------
add_subdirectory(chapter00_hello)
add_subdirectory(chapter01_camera_view)
add_subdirectory(chapter02_color_tracker)
add_subdirectory(chapter03_osc_sender)
add_subdirectory(chapter04_detector_abstraction)
add_subdirectory(chapter05_optical_flow)
add_subdirectory(chapter06_async_pipeline)
add_subdirectory(chapter07_config)
add_subdirectory(chapter08_pose_estimation)
add_subdirectory(chapter09_osc_receive)
