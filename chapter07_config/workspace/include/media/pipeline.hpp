// ============================================================
// media/pipeline.hpp
// ------------------------------------------------------------
// Capture → Detect → Send の 3 スレッドパイプライン。
//
// この章で学ぶ主要概念:
//   - std::jthread: C++20 の join 保証付きスレッド
//   - std::stop_token: 協調的な停止メカニズム
//   - SpscQueue: スレッド間通信バッファ
//
// TS 対比:
//   TS のワーカースレッドは基本的に SharedArrayBuffer + Atomics。
//   C++ では std::thread / std::jthread が OS スレッドを直接ラップする。
//   std::jthread は「スコープを外れたとき自動 join」= RAII のスレッド版。
// ============================================================
#pragma once

#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"
#include "media/spsc_queue.hpp"
#include "media/osc_sender.hpp"
#include "media/osc_value.hpp"

#include <atomic>
#include <functional>
#include <thread>

#include <opencv2/core.hpp>

namespace media {

// ============================================================
// Pipeline — 3 スレッドパイプライン
// ------------------------------------------------------------
// スレッド 1 (Capture): カメラからフレームを取得 → frame_queue に push
// スレッド 2 (Detect) : frame_queue から取り出し → 検出 → result_queue に push
// スレッド 3 (Send)   : result_queue から取り出し → OSC 送信
// ============================================================
class Pipeline {
public:
    // キューの容量（固定）
    static constexpr std::size_t kQueueSize = 8;

    Pipeline(CameraCapture& cam, ColorTracker& tracker, OscSender& osc_sender);

    // コピー禁止（スレッドは移動不可）
    Pipeline(const Pipeline&)            = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline(Pipeline&&)                 = delete;
    Pipeline& operator=(Pipeline&&)      = delete;

    // デストラクタ: jthread は自動で stop() → join() を呼ぶ (RAII)
    ~Pipeline() = default;

    // パイプライン開始
    void start();

    // パイプライン停止（stop_source に停止要求を送り、jthread が join するのを待つ）
    void stop();

    [[nodiscard]] bool is_running() const;

private:
    // 参照で保持（Pipeline は所有しない）
    CameraCapture& cam_;
    ColorTracker&  tracker_;
    OscSender&     osc_sender_;

    // スレッド間キュー
    SpscQueue<cv::Mat, kQueueSize> frame_queue_;
    SpscQueue<DetectorOutput, kQueueSize> result_queue_;

    // std::jthread: スコープを出ると自動的に stop + join（RAII）
    // C++20 の std::jthread は std::thread の「改良版」。
    // std::thread はデストラクタで join を呼ばないと std::terminate になる。
    std::jthread capture_thread_;
    std::jthread detect_thread_;
    std::jthread send_thread_;

    std::atomic<bool> running_{false};

    // 各スレッドの処理関数
    // std::stop_token: 協調的停止のための仕組み
    //   stop_requested() が true になったらループを抜ける。
    //   例外を投げる強制終了と違い、リソースを安全に解放してから停止できる。
    void capture_loop(std::stop_token token);
    void detect_loop(std::stop_token token);
    void send_loop(std::stop_token token);
};

} // namespace media
