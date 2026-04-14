// ============================================================
// src/color_tracker.cpp
// ------------------------------------------------------------
// HSV 色域トラッカーの実装。
//
// 処理フロー:
//   BGR 画像 → HSV 変換 → 色域マスク生成 → モーメント計算 → 重心・面積
//
// OpenCV の主な関数:
//   cv::cvtColor  : 色空間変換（BGR→HSV）
//   cv::inRange   : 色域内のピクセルを 1 にするマスク生成
//   cv::moments   : 2値画像のモーメント（重心計算に使う）
// ============================================================
#include "media/color_tracker.hpp"

#include <opencv2/imgproc.hpp>

namespace media {

std::optional<TrackResult> track(const cv::Mat& bgr, const HsvRange& range) {
    // -------------------------------------------------------
    // 1. 入力チェック
    // -------------------------------------------------------
    // 空の行列（open 失敗時など）は処理しない。
    if (bgr.empty()) {
        return std::nullopt;
    }

    // -------------------------------------------------------
    // 2. BGR → HSV 変換
    // -------------------------------------------------------
    // カメラは BGR 形式で画像を返す（歴史的経緯で BGR 順）。
    // HSV は「色相・彩度・明度」で色を表現するため、
    // 照明の変化（明度の変化）に強く、色の絞り込みに適している。
    //
    // TS 対比: 副作用のない純粋変換。hsv は新しい Mat（immutable 的）。
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    // -------------------------------------------------------
    // 3. 色域マスク生成
    // -------------------------------------------------------
    // HsvRange で指定した下限・上限の範囲内のピクセルを
    // 255 (白)、範囲外を 0 (黒) にした2値画像を生成する。
    cv::Mat mask;
    cv::inRange(hsv, range.lower, range.upper, mask);

    // -------------------------------------------------------
    // 4. モーメント計算 → 重心・面積
    // -------------------------------------------------------
    // cv::moments は画像の「モーメント（統計量）」を計算する。
    // m00 = 面積（白ピクセル数）
    // m10 / m00 = x 重心, m01 / m00 = y 重心
    //
    // binaryImage = true: マスク画像（0/255）を 0/1 として計算
    cv::Moments m = cv::moments(mask, /*binaryImage=*/true);

    // 面積がほぼゼロなら検出失敗とみなす。
    // ゼロ除算防止と「ノイズ」の除去を兼ねる。
    const double min_area = 100.0; // 100 ピクセル未満は除外
    if (m.m00 < min_area) {
        return std::nullopt;
    }

    // 重心座標を計算して TrackResult に格納して返す。
    TrackResult result;
    result.centroid = cv::Point2f(
        static_cast<float>(m.m10 / m.m00), // x 座標
        static_cast<float>(m.m01 / m.m00)  // y 座標
    );
    result.area = m.m00;

    return result;
}

} // namespace media
