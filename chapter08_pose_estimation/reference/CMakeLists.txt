# ============================================================
# Chapter 08 reference (お手本)
# ------------------------------------------------------------
# ONNXRuntime はオプション（デフォルト OFF）。
# ============================================================

# PoseEstimator ライブラリ
add_library(chapter08_ref_pose
    src/pose_estimator.cpp
)

target_include_directories(chapter08_ref_pose PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(chapter08_ref_pose PUBLIC cxx_std_20)
target_link_libraries(chapter08_ref_pose PUBLIC ${OpenCV_LIBS})
target_include_directories(chapter08_ref_pose PUBLIC ${OpenCV_INCLUDE_DIRS})

# ONNXRuntime の検索（オプション）
if(MODERN_CPP_ENABLE_ONNX)
    find_package(onnxruntime QUIET)
    if(onnxruntime_FOUND)
        target_link_libraries(chapter08_ref_pose PUBLIC onnxruntime::onnxruntime)
        target_compile_definitions(chapter08_ref_pose PRIVATE MODERN_CPP_WITH_ONNX)
        message(STATUS "Chapter 08: ONNXRuntime が見つかりました。有効化します。")
    else()
        message(WARNING "Chapter 08: -DMODERN_CPP_ENABLE_ONNX=ON が指定されましたが "
                        "onnxruntime が見つかりません。スタブ実装を使います。"
                        " brew install onnxruntime を実行してください。")
    endif()
endif()

modern_cpp_enable_warnings(chapter08_ref_pose)

# テスト
add_executable(chapter08_ref_tests
    tests/test_pose_estimator.cpp
)
target_link_libraries(chapter08_ref_tests PRIVATE
    chapter08_ref_pose
    doctest::doctest
)
modern_cpp_enable_warnings(chapter08_ref_tests)

include(doctest)
doctest_discover_tests(chapter08_ref_tests)
