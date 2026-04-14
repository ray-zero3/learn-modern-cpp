// ============================================================
// src/color_tracker.cpp (Chapter 04 更新版)
// ------------------------------------------------------------
// ColorTracker クラスの実装。
// Chapter 02 の track() 関数をクラスにラップし、
// Detector concept を満たす process() を追加する。
// ============================================================
#include "media/color_tracker.hpp"

#include <opencv2/imgproc.hpp>

namespace media {

// ============================================================
// track 関数 (Chapter 02 から引き続き提供)
// ============================================================
std::optional<TrackResult> track(const cv::Mat& bgr, const HsvRange& range) {
    if (bgr.empty()) {
        return std::nullopt;
    }

    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(hsv, range.lower, range.upper, mask);

    cv::Moments m = cv::moments(mask, /*binaryImage=*/true);

    const double min_area = 100.0;
    if (m.m00 < min_area) {
        return std::nullopt;
    }

    TrackResult result;
    result.centroid = cv::Point2f(
        static_cast<float>(m.m10 / m.m00),
        static_cast<float>(m.m01 / m.m00)
    );
    result.area = m.m00;

    return result;
}

// ============================================================
// ColorTracker クラスの実装
// ============================================================

ColorTracker::ColorTracker(HsvRange range) : range_(std::move(range)) {}

// -------------------------------------------------------
// process: Detector concept を満たすメソッド
// -------------------------------------------------------
// 内部で track() 関数を呼び出し、
// TrackResult を DetectorOutput (variant) に包んで返す。
//
// なぜ variant でラップするか:
//   Detector concept は「型を問わず process() を呼べる」を保証したい。
//   ColorTracker も OpticalFlowDetector も同じ型 (DetectorOutput) を返すことで、
//   呼び出し側が型を知らなくてもよくなる。
std::optional<DetectorOutput> ColorTracker::process(const cv::Mat& frame) {
    auto result = track(frame, range_);
    if (!result.has_value()) {
        return std::nullopt;
    }
    // TrackResult を DetectorOutput (variant) に変換して返す。
    // std::variant は代入側の型で構築できる（暗黙変換）。
    return DetectorOutput{*result};
}

void ColorTracker::set_range(HsvRange range) {
    range_ = std::move(range);
}

const HsvRange& ColorTracker::range() const {
    return range_;
}

} // namespace media
