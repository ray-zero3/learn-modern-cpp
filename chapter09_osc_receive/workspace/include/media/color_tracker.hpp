// ============================================================
// media/color_tracker.hpp (Chapter 04 更新版)
// ------------------------------------------------------------
// Chapter 02 から変更点:
//   - ColorTracker クラスに process() メソッドを追加
//   - Detector concept を満たすようにする
//
// Detector concept が要求するのは:
//   process(const cv::Mat&) -> std::optional<DetectorOutput>
//
// TS 対比:
//   - HsvRange は TS の interface に相当するが、値型として扱う
//   - const cv::Mat& ≈ Readonly<Mat>（所有しないビュー）
//   - ColorTracker implements Detector に相当するが、
//     C++ では「implements」キーワードなしに concept を満たせる
// ============================================================
#pragma once

#include <optional>
#include <variant>

#include <opencv2/core.hpp>

namespace media {

// ============================================================
// HsvRange — HSV 色空間での色域指定
// ============================================================
struct HsvRange {
    cv::Scalar lower; // HSV 下限値
    cv::Scalar upper; // HSV 上限値
};

// ============================================================
// TrackResult — 追跡結果
// ============================================================
struct TrackResult {
    cv::Point2f centroid; // 重心座標 (pixel, float 精度)
    double area;          // 検出領域の面積 (pixel^2)
};

// FlowVec の前方宣言（detector.hpp / optical_flow.hpp で定義される）
// ここでは variant 型に使うためだけに前方宣言する。
struct FlowVec {
    float dx;
    float dy;
    float magnitude;
};

// DetectorOutput: 全 Detector の出力を統一する variant 型
// TrackResult または FlowVec を保持できる。
using DetectorOutput = std::variant<TrackResult, FlowVec>;

// ============================================================
// 関数 track (Chapter 02 から継続)
// ============================================================
[[nodiscard]] std::optional<TrackResult> track(const cv::Mat& bgr,
                                                const HsvRange& range);

// ============================================================
// ColorTracker — Detector concept を満たすクラス
// ------------------------------------------------------------
// 従来の track() 関数をクラスにラップし、
// Detector concept が要求する process() インターフェースを提供する。
//
// TS 対比:
//   class ColorTracker implements Detector { ... } に相当するが、
//   C++ では「implements」キーワードなしに concept を満たせる。
//   concept は「鴨型付け（duck typing）」をコンパイル時に検証する。
// ============================================================
class ColorTracker {
public:
    // コンストラクタ: HSV 範囲を設定
    explicit ColorTracker(HsvRange range);

    // -------------------------------------------------------
    // process: Detector concept を満たすメソッド
    // -------------------------------------------------------
    // 戻り値の型は DetectorOutput（variant<TrackResult, FlowVec>）。
    // TrackResult が検出されたら DetectorOutput に包んで返す。
    // 検出失敗時は std::nullopt。
    [[nodiscard]] std::optional<DetectorOutput> process(const cv::Mat& frame);

    // HSV 範囲を更新する（動的なパラメータ変更用）
    void set_range(HsvRange range);

    [[nodiscard]] const HsvRange& range() const;

private:
    HsvRange range_;
};

} // namespace media
