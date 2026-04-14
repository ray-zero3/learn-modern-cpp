// ============================================================
// media/pose_estimator.hpp
// ------------------------------------------------------------
// ONNXRuntime を使ったポーズ推定クラスの骨格実装。
//
// この章は Optional（ヘビー依存のため）。
// MODERN_CPP_ENABLE_ONNX=ON の場合のみビルドされる。
//
// ビルド方法:
//   cmake -S . -B build -DMODERN_CPP_ENABLE_ONNX=ON ...
//   brew install onnxruntime が必要
//
// この章で学ぶ主要概念:
//   - C API との相互運用（ONNXRuntime は C API を持つ）
//   - RAII でハンドルを管理（C API のクリーンアップを自動化）
//   - std::span<const float> でテンソルデータを扱う
//
// TS 対比:
//   ONNX.js / onnxruntime-web は JS インターフェースを提供するが、
//   C++ では C API を直接呼ぶのが一般的。ポインタとハンドルの管理が必要。
//
// 深掘り: docs/deepdive/08-c-api-interop.md
// ============================================================
#pragma once

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <opencv2/core.hpp>

namespace media {

// ============================================================
// KeyPoint — 関節点（ポーズ推定の出力）
// ============================================================
struct KeyPoint {
    float x;          // 正規化座標 (0.0-1.0)
    float y;          // 正規化座標 (0.0-1.0)
    float confidence; // 信頼度 (0.0-1.0)
};

// 17 関節点（MoveNet の出力形式）
static constexpr int kNumKeyPoints = 17;
using PoseResult = std::array<KeyPoint, kNumKeyPoints>;

// ============================================================
// PoseEstimator — ONNX モデルを使ったポーズ推定
// ------------------------------------------------------------
// ONNXRuntime が利用できない場合でも、クラスのインターフェースは
// コンパイル可能にしておく（ONNX依存はソースファイルに閉じ込める）。
// ============================================================
class PoseEstimator {
public:
    PoseEstimator();
    ~PoseEstimator();

    // コピー禁止（ONNXRuntime セッションは複製できない）
    PoseEstimator(const PoseEstimator&)            = delete;
    PoseEstimator& operator=(const PoseEstimator&) = delete;

    // ムーブ可能
    PoseEstimator(PoseEstimator&&)            = default;
    PoseEstimator& operator=(PoseEstimator&&) = default;

    // -------------------------------------------------------
    // load: ONNXモデルを読み込む
    // -------------------------------------------------------
    // モデルファイルが存在しない場合や ONNX が無い場合は false を返す。
    // 例外を投げずに失敗を bool で返す設計（Chapter 7 の期待に合わせる）。
    [[nodiscard]] bool load(std::string_view model_path);

    // -------------------------------------------------------
    // estimate: ポーズを推定する
    // -------------------------------------------------------
    // bgr: BGR 形式のカメラフレーム
    // 戻り値: 推定成功時は PoseResult、失敗時は nullopt
    [[nodiscard]] std::optional<PoseResult> estimate(const cv::Mat& bgr);

    [[nodiscard]] bool is_loaded() const { return loaded_; }

private:
    bool loaded_{false};

    // ONNXRuntime のセッション（実装はソースファイルに閉じ込める）
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace media
