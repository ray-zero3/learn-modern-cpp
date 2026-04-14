# ============================================================
# Chapter 08: ONNXRuntime ポーズ推定 (Optional)
# ============================================================

# デフォルト OFF。有効化するには -DMODERN_CPP_ENABLE_ONNX=ON を指定。
option(MODERN_CPP_ENABLE_ONNX "Chapter 8 の ONNX を有効化" OFF)

if(NOT MODERN_CPP_ENABLE_ONNX)
    message(STATUS "Chapter 08: スキップ (MODERN_CPP_ENABLE_ONNX=OFF)")
    message(STATUS "  有効化するには: cmake ... -DMODERN_CPP_ENABLE_ONNX=ON")
    message(STATUS "  依存: brew install onnxruntime")
    # OFF の場合もスタブ実装でテストが通るように reference だけビルドする
endif()

# ONNX の有無に関わらず reference をビルド（スタブ実装でテストが通る）
if(MODERN_CPP_BUILD_REFERENCE)
    add_subdirectory(reference)
endif()

if(MODERN_CPP_BUILD_WORKSPACE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/workspace/CMakeLists.txt")
    add_subdirectory(workspace)
endif()
