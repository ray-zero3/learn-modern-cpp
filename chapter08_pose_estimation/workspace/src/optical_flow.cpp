// ============================================================
// src/optical_flow.cpp
// ------------------------------------------------------------
// OpticalFlowDetector の実装。
//
// Farneback 法 dense optical flow:
//   cv::calcOpticalFlowFarneback(prev, next, flow, ...)
//   flow は各ピクセルの (dx, dy) ベクトルを持つ CV_32FC2 行列
// ============================================================
#include "media/optical_flow.hpp"
#include "media/detector.hpp"

#include <cmath>

#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

namespace media {

// Detector concept を満たすこと静的に確認
static_assert(Detector<OpticalFlowDetector>,
    "OpticalFlowDetector は Detector concept を満たさなければならない");

// ============================================================
// process: フロー計算
// ============================================================
std::optional<DetectorOutput> OpticalFlowDetector::process(const cv::Mat& frame) {
    if (frame.empty()) {
        return std::nullopt;
    }

    // BGR → グレースケール変換（フロー計算はグレースケールで行う）
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // 前フレームがない場合は保存して終了（初回フレーム）
    if (!prev_gray_.has_value()) {
        prev_gray_ = gray.clone();
        return std::nullopt;
    }

    // -------------------------------------------------------
    // Farneback 法でオプティカルフローを計算する
    // -------------------------------------------------------
    // flow: CV_32FC2 (float, 2チャンネル) の行列
    //   各ピクセルの [dx, dy] が入っている
    //
    // パラメータ:
    //   pyr_scale = 0.5  : ピラミッドスケール（0.5 = 半分ずつ小さくする）
    //   levels = 3       : ピラミッドのレベル数
    //   winsize = 15     : 平均化ウィンドウサイズ
    //   iterations = 3   : 各レベルの反復回数
    //   poly_n = 5       : 多項式展開の近傍サイズ
    //   poly_sigma = 1.2 : ガウス標準偏差
    cv::Mat flow;
    cv::calcOpticalFlowFarneback(
        *prev_gray_, gray, flow,
        0.5, 3, 15, 3, 5, 1.2, 0
    );

    // 前フレームを更新（新しいグレースケール画像をコピーして保存）
    prev_gray_ = gray.clone();

    // -------------------------------------------------------
    // フロー行列から平均ベクトルを計算
    // -------------------------------------------------------
    // flow の各ピクセルは cv::Point2f (dx, dy)。
    // 全ピクセルの平均を取ることで「全体的な動き」を得る。
    //
    // std::array<float, 2> を使えばスタック割り当て（学習ポイント）だが、
    // ここでは可読性を優先して cv::Scalar を使う。
    cv::Scalar mean_flow = cv::mean(flow);
    const float dx = static_cast<float>(mean_flow[0]); // x方向の平均移動
    const float dy = static_cast<float>(mean_flow[1]); // y方向の平均移動
    const float magnitude = std::sqrt(dx * dx + dy * dy);

    // 動きが最小閾値以下なら「静止」として nullopt を返す
    if (magnitude < min_magnitude_) {
        return std::nullopt;
    }

    return DetectorOutput{FlowVec{dx, dy, magnitude}};
}

// ============================================================
// reset: 前フレームをクリア
// ============================================================
void OpticalFlowDetector::reset() {
    prev_gray_.reset(); // optional をクリア
}

// ============================================================
// set_min_magnitude
// ============================================================
void OpticalFlowDetector::set_min_magnitude(float min_mag) {
    min_magnitude_ = min_mag;
}

} // namespace media
