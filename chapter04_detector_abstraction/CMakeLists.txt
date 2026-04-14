# ============================================================
# Chapter 04: Detector 抽象化 (concepts)
# ============================================================

if(MODERN_CPP_BUILD_REFERENCE)
    add_subdirectory(reference)
endif()

if(MODERN_CPP_BUILD_WORKSPACE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/workspace/CMakeLists.txt")
    add_subdirectory(workspace)
endif()
