// ============================================================
// tests/test_color_tracker.cpp
// ------------------------------------------------------------
// HSV色域トラッカーの単体テスト。
//
// テスト設計の方針:
//   実カメラを使わず、「合成画像（既知の色・形）」で検証する。
//   既知の重心と面積を計算して結果と照合できる。
//
// 合成画像戦略:
//   - 真っ黒な背景に、特定の BGR 色の矩形を描画
//   - その色に対応する HSV 範囲でトラックし、
//     重心が矩形中心に一致するか・面積が正しいかを検証
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/color_tracker.hpp"

#include <opencv2/imgproc.hpp>

// -------------------------------------------------------
// ヘルパー: 指定色の矩形を持つ合成画像を作成
// -------------------------------------------------------
// image_size : 画像全体のサイズ (width, height)
// rect       : 矩形の位置とサイズ
// bgr_color  : 矩形の色（BGR スカラー）
static cv::Mat make_test_image(cv::Size image_size, cv::Rect rect, cv::Scalar bgr_color) {
    // 黒で初期化した画像を作る。
    // cv::Mat(rows, cols, type, scalar) のコンストラクタ。
    // CV_8UC3: 8bit unsigned, 3チャンネル（BGR）
    cv::Mat img(image_size.height, image_size.width, CV_8UC3, cv::Scalar(0, 0, 0));
    // 矩形領域を指定色で塗りつぶす
    cv::rectangle(img, rect, bgr_color, cv::FILLED);
    return img;
}

TEST_CASE("track: 空の画像では nullopt を返す") {
    media::HsvRange range{cv::Scalar(0, 0, 0), cv::Scalar(179, 255, 255)};
    cv::Mat empty;
    auto result = media::track(empty, range);
    CHECK(result.has_value() == false);
}

TEST_CASE("track: 色が存在しない画像では nullopt を返す") {
    // 真っ黒な画像（何も検出されないはず）
    cv::Mat black(100, 100, CV_8UC3, cv::Scalar(0, 0, 0));
    // 赤系の HSV 範囲（OpenCV での赤1: H=0-10）
    media::HsvRange red_range{cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255)};
    auto result = media::track(black, red_range);
    CHECK(result.has_value() == false);
}

TEST_CASE("track: 既知の矩形から重心が正しく計算される") {
    // 200x200 の画像に、(50,50) - (150,150) の青い矩形を描く。
    // 純青 BGR = (255, 0, 0) → HSV ≈ (120, 255, 255)
    const cv::Rect blue_rect(50, 50, 100, 100);
    cv::Mat img = make_test_image(cv::Size(200, 200), blue_rect, cv::Scalar(255, 0, 0));

    // 青の HSV 範囲: H=100-130, S=100-255, V=100-255
    media::HsvRange blue_range{
        cv::Scalar(100, 100, 100),
        cv::Scalar(130, 255, 255)
    };

    auto result = media::track(img, blue_range);

    // 検出できたこと
    REQUIRE(result.has_value() == true);

    // 矩形 (50,50)-(150,150) の重心は (100, 100) のはず。
    // 浮動小数点の誤差を許容して ±2 pixel 以内で確認。
    CHECK(result->centroid.x == doctest::Approx(100.0F).epsilon(0.02));
    CHECK(result->centroid.y == doctest::Approx(100.0F).epsilon(0.02));

    // 面積は 100*100 = 10000 ピクセル（±5% 程度の誤差を許容）
    CHECK(result->area == doctest::Approx(10000.0).epsilon(0.05));
}

TEST_CASE("track: 面積が小さすぎる場合は nullopt") {
    // 5x5 の小さな矩形（最小面積 100px 未満）
    const cv::Rect tiny_rect(50, 50, 5, 5);
    cv::Mat img = make_test_image(cv::Size(200, 200), tiny_rect, cv::Scalar(255, 0, 0));

    media::HsvRange blue_range{
        cv::Scalar(100, 100, 100),
        cv::Scalar(130, 255, 255)
    };

    auto result = media::track(img, blue_range);
    // 面積 25px < 100px なので nullopt が返るはず
    CHECK(result.has_value() == false);
}

TEST_CASE("track: 緑色の矩形が正しく追跡される") {
    // 緑 BGR = (0, 255, 0) → HSV ≈ (60, 255, 255)
    const cv::Rect green_rect(20, 30, 80, 60);
    cv::Mat img = make_test_image(cv::Size(200, 200), green_rect, cv::Scalar(0, 255, 0));

    // 緑の HSV 範囲
    media::HsvRange green_range{
        cv::Scalar(50, 100, 100),
        cv::Scalar(70, 255, 255)
    };

    auto result = media::track(img, green_range);
    REQUIRE(result.has_value() == true);

    // 矩形 (20,30)-(100,90) の重心: x=60, y=60
    CHECK(result->centroid.x == doctest::Approx(60.0F).epsilon(0.02));
    CHECK(result->centroid.y == doctest::Approx(60.0F).epsilon(0.02));
}
