// ============================================================
// tests/test_pose_estimator.cpp
// ------------------------------------------------------------
// PoseEstimator の単体テスト。
// ONNXRuntime の有無に関わらずテストが通ること。
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/pose_estimator.hpp"

TEST_CASE("PoseEstimator: デフォルト構築後は is_loaded が false") {
    media::PoseEstimator estimator;
    CHECK(estimator.is_loaded() == false);
}

TEST_CASE("PoseEstimator: 存在しないモデルファイルの load は false を返す") {
    media::PoseEstimator estimator;
    // モデルファイルが存在しない（ONNXRuntime の有無に関わらず false）
    bool ok = estimator.load("/tmp/nonexistent_model.onnx");
    CHECK(ok == false);
    CHECK(estimator.is_loaded() == false);
}

TEST_CASE("PoseEstimator: is_loaded が false の場合 estimate は nullopt を返す") {
    media::PoseEstimator estimator;
    cv::Mat dummy(192, 192, CV_8UC3, cv::Scalar(128, 128, 128));
    auto result = estimator.estimate(dummy);
    CHECK(result.has_value() == false);
}

TEST_CASE("PoseEstimator: 空の画像で estimate は nullopt を返す") {
    media::PoseEstimator estimator;
    cv::Mat empty;
    auto result = estimator.estimate(empty);
    CHECK(result.has_value() == false);
}
