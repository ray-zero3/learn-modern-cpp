# ============================================================
# Chapter 00: C++ プロジェクトの 0→1
# ------------------------------------------------------------
# reference/ と workspace/ をそれぞれ独立してビルドする。
# 名前空間衝突を避けるため、各ターゲットには `chapter00_` の
# 接頭辞を付ける（CMake にはネームスペースがないため）。
# ============================================================

if(MODERN_CPP_BUILD_REFERENCE)
    add_subdirectory(reference)
endif()

# workspace は「あなたが一から作る場所」。この章に限っては開始時点で
# 空なので、CMakeLists.txt を自分で用意するまでは add_subdirectory を
# スキップする。ファイルを作り始めたら自動的にビルド対象に入る。
if(MODERN_CPP_BUILD_WORKSPACE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/workspace/CMakeLists.txt")
    add_subdirectory(workspace)
endif()
