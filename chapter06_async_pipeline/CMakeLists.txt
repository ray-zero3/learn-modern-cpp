# ============================================================
# Chapter 06: 非同期パイプライン
# ============================================================

if(MODERN_CPP_BUILD_REFERENCE)
    add_subdirectory(reference)
endif()

if(MODERN_CPP_BUILD_WORKSPACE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/workspace/CMakeLists.txt")
    add_subdirectory(workspace)
endif()
