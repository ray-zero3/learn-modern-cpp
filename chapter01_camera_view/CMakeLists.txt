# ============================================================
# Chapter 01: カメラ映像をウィンドウに表示
# ------------------------------------------------------------
# reference と workspace をそれぞれ独立してビルドする。
# ============================================================

if(MODERN_CPP_BUILD_REFERENCE)
    add_subdirectory(reference)
endif()

# workspace は前章 (chapter00) の reference を開始点とする。
# ファイルが存在する場合のみ追加する。
if(MODERN_CPP_BUILD_WORKSPACE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/workspace/CMakeLists.txt")
    add_subdirectory(workspace)
endif()
