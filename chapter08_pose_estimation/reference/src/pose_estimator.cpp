// ============================================================
// src/pose_estimator.cpp
// ------------------------------------------------------------
// PoseEstimator の実装。
//
// MODERN_CPP_WITH_ONNX が定義されている場合のみ ONNXRuntime を使う。
// 未定義の場合はスタブ実装（常に false / nullopt を返す）。
// ============================================================
#include "media/pose_estimator.hpp"

#ifdef MODERN_CPP_WITH_ONNX
// ONNXRuntime の C API ヘッダ
// brew install onnxruntime でインストールされる
#include <onnxruntime/core/providers/providers.h>
#include <onnxruntime_cxx_api.h>
#endif

#include <iostream>

namespace media {

#ifdef MODERN_CPP_WITH_ONNX

// ============================================================
// ONNXRuntime 有効時の実装
// ============================================================
struct PoseEstimator::Impl {
    Ort::Env env;
    Ort::Session session{nullptr};
    Ort::SessionOptions session_options;

    Impl() : env(ORT_LOGGING_LEVEL_WARNING, "PoseEstimator") {
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
    }
};

PoseEstimator::PoseEstimator() : impl_(std::make_unique<Impl>()) {}
PoseEstimator::~PoseEstimator() = default;

bool PoseEstimator::load(std::string_view model_path) {
    try {
        std::string path_str(model_path);
        impl_->session = Ort::Session(impl_->env, path_str.c_str(), impl_->session_options);
        loaded_ = true;
        std::cout << "[PoseEstimator] モデル読み込み完了: " << path_str << '\n';
        return true;
    } catch (const Ort::Exception& e) {
        std::cerr << "[PoseEstimator] モデル読み込み失敗: " << e.what() << '\n';
        loaded_ = false;
        return false;
    }
}

std::optional<PoseResult> PoseEstimator::estimate(const cv::Mat& bgr) {
    if (!loaded_) return std::nullopt;
    if (bgr.empty()) return std::nullopt;

    // TODO: 実際の推論処理
    // MoveNet の入力: [1, 192, 192, 3] (RGB, float32, 0.0-1.0 正規化)
    // 出力: [1, 1, 17, 3] (y, x, confidence)
    // この章の骨格実装ではスタブを返す。

    PoseResult result;
    for (auto& kp : result) {
        kp = {0.5F, 0.5F, 0.0F}; // スタブ: すべて中央、信頼度0
    }
    return result;
}

#else

// ============================================================
// ONNXRuntime 無効時のスタブ実装
// ============================================================
// MODERN_CPP_ENABLE_ONNX=OFF の場合（デフォルト）。
// クラスはコンパイルできるが、常に「ロード失敗」を返す。
// テストは「モデルファイルがない場合 load が false を返すこと」で十分。

struct PoseEstimator::Impl {};

PoseEstimator::PoseEstimator() : impl_(std::make_unique<Impl>()) {}
PoseEstimator::~PoseEstimator() = default;

bool PoseEstimator::load(std::string_view model_path) {
    std::cerr << "[PoseEstimator] ONNXRuntime が無効です。"
              << " -DMODERN_CPP_ENABLE_ONNX=ON でビルドしてください。\n"
              << " モデルパス: " << model_path << '\n';
    loaded_ = false;
    return false;
}

std::optional<PoseResult> PoseEstimator::estimate([[maybe_unused]] const cv::Mat& bgr) {
    return std::nullopt;
}

#endif // MODERN_CPP_WITH_ONNX

} // namespace media
