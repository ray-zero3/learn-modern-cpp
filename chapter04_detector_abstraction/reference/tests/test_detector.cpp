// ============================================================
// tests/test_detector.cpp
// ------------------------------------------------------------
// Detector concept と ColorTracker の単体テスト。
//
// テスト内容:
//   1. ColorTracker が Detector concept を満たすこと（静的テスト）
//   2. ColorTracker::process() が正しい結果を返すこと（動的テスト）
//   3. Detector を満たさない型が static_assert で検出されること（コメントアウトで示す）
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/detector.hpp"
#include "media/color_tracker.hpp"

#include <opencv2/imgproc.hpp>

// ============================================================
// 静的テスト: ColorTracker が Detector を満たすこと
// ============================================================
// static_assert はコンパイル時に評価される。
// 満たさない場合はビルドが失敗する（= コンパイル時テスト）。
//
// TS でいう型チェック (`tsc --strict`) をコンパイル時に行うイメージ。
static_assert(media::Detector<media::ColorTracker>,
    "ColorTracker は Detector concept を満たさなければならない");

// ============================================================
// Detector を満たさない型の例（コメントアウト）
// ------------------------------------------------------------
// struct NotADetector { void foo() {} };
// static_assert(media::Detector<NotADetector>);
//   → コンパイルエラー: "NotADetector does not satisfy Detector"
// ============================================================

// ============================================================
// ヘルパー: テスト用合成画像の作成
// ============================================================
static cv::Mat make_test_image(cv::Size size, cv::Rect rect, cv::Scalar bgr_color) {
    cv::Mat img(size.height, size.width, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(img, rect, bgr_color, cv::FILLED);
    return img;
}

// ============================================================
// 動的テスト: ColorTracker::process()
// ============================================================

TEST_CASE("ColorTracker: process が DetectorOutput (TrackResult) を返す") {
    // 青い矩形を含む合成画像
    const cv::Rect rect(50, 50, 100, 100);
    cv::Mat img = make_test_image(cv::Size(200, 200), rect, cv::Scalar(255, 0, 0));

    media::ColorTracker tracker{media::HsvRange{
        cv::Scalar(100, 100, 100),
        cv::Scalar(130, 255, 255)
    }};

    // process() を呼ぶ
    auto result = tracker.process(img);

    // 検出成功
    REQUIRE(result.has_value() == true);

    // 戻り値が TrackResult を保持していること
    // std::get<TrackResult> で取り出せる（型が違えば例外）
    const auto* track_result = std::get_if<media::TrackResult>(&result.value());
    REQUIRE(track_result != nullptr);

    // 重心が正しいこと
    CHECK(track_result->centroid.x == doctest::Approx(100.0F).epsilon(0.02));
    CHECK(track_result->centroid.y == doctest::Approx(100.0F).epsilon(0.02));
}

TEST_CASE("ColorTracker: process が空の画像では nullopt を返す") {
    media::ColorTracker tracker{media::HsvRange{
        cv::Scalar(100, 100, 100),
        cv::Scalar(130, 255, 255)
    }};

    cv::Mat empty;
    auto result = tracker.process(empty);
    CHECK(result.has_value() == false);
}

TEST_CASE("ColorTracker: set_range で HSV 範囲を変更できる") {
    media::ColorTracker tracker{media::HsvRange{
        cv::Scalar(100, 100, 100), // 青
        cv::Scalar(130, 255, 255)
    }};

    // 緑の矩形
    cv::Mat green_img = make_test_image(
        cv::Size(200, 200),
        cv::Rect(50, 50, 100, 100),
        cv::Scalar(0, 255, 0)
    );

    // 青設定では緑は検出されない
    auto result1 = tracker.process(green_img);
    CHECK(result1.has_value() == false);

    // 緑の範囲に変更
    tracker.set_range(media::HsvRange{
        cv::Scalar(50, 100, 100),
        cv::Scalar(70, 255, 255)
    });

    // 緑が検出されるようになる
    auto result2 = tracker.process(green_img);
    CHECK(result2.has_value() == true);
}

TEST_CASE("DetectorOutput: std::visit で TrackResult を取り出せる") {
    // DetectorOutput は variant<TrackResult, FlowVec>
    // std::visit で型に応じた処理ができることを確認

    media::DetectorOutput output = media::TrackResult{
        cv::Point2f(100.0F, 100.0F),
        10000.0
    };

    // std::visit でどちらの型かを判定して処理する
    std::string type_name;
    std::visit([&type_name](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, media::TrackResult>) {
            type_name = "TrackResult";
        } else if constexpr (std::is_same_v<T, media::FlowVec>) {
            type_name = "FlowVec";
        }
    }, output);

    CHECK(type_name == "TrackResult");
}
