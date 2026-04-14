// ============================================================
// media/optical_flow.hpp
// ------------------------------------------------------------
// オプティカルフロー検出器の宣言。
// Farneback 法（dense optical flow）でフレーム間の動きを検出する。
//
// この章で学ぶ主要概念:
//   - OpticalFlowDetector が Detector concept を満たすこと
//   - std::array<T, N>: スタック割り当ての固定サイズ配列
//   - std::vector vs std::array の使い分け
//
// TS 対比:
//   - std::array は TypedArray (Float32Array 等) に近い
//   - ヒープ割り当て不要 = メモリアロケーションのオーバーヘッドなし
//
// 深掘り: docs/deepdive/05-stack-vs-heap-cache.md
// ============================================================
#pragma once

#include "media/color_tracker.hpp" // DetectorOutput, FlowVec

#include <optional>

#include <opencv2/core.hpp>

namespace media {

// ============================================================
// OpticalFlowDetector — Farneback 法オプティカルフロー検出器
// ------------------------------------------------------------
// 前フレームを保持し、現フレームとの差分（動きベクトル）を計算する。
// 動きベクトルの平均値を FlowVec として返す。
//
// Farneback 法:
//   - 全ピクセルに対してフロー計算（dense）
//   - Lucas-Kanade（sparse）より計算コストが高いが情報量が多い
// ============================================================
class OpticalFlowDetector {
public:
    OpticalFlowDetector() = default;

    // コピー禁止（前フレームバッファの二重所有を防ぐ）
    OpticalFlowDetector(const OpticalFlowDetector&)            = delete;
    OpticalFlowDetector& operator=(const OpticalFlowDetector&) = delete;

    // ムーブ可能
    OpticalFlowDetector(OpticalFlowDetector&&)            = default;
    OpticalFlowDetector& operator=(OpticalFlowDetector&&) = default;

    // -------------------------------------------------------
    // process: Detector concept を満たすメソッド
    // -------------------------------------------------------
    // 最初のフレームは前フレームとして保存するだけ（nullopt を返す）。
    // 2フレーム目以降からフロー計算を開始する。
    [[nodiscard]] std::optional<DetectorOutput> process(const cv::Mat& frame);

    // 検出器をリセット（前フレームをクリア）
    void reset();

    // 最小フロー大きさ（これ以下は「静止」として nullopt を返す）
    void set_min_magnitude(float min_mag);

private:
    // 前フレーム（グレースケール）を保持する。
    // optional で「初回フレームがまだない」状態を表現する。
    std::optional<cv::Mat> prev_gray_;

    // 最小フロー大きさ（ノイズ除去用）
    float min_magnitude_{1.0F};
};

} // namespace media
