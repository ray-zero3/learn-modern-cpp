// ============================================================
// src/pipeline.cpp
// ------------------------------------------------------------
// Pipeline の実装。
// ============================================================
#include "media/pipeline.hpp"

#include <chrono>
#include <iostream>
#include <vector>

namespace media {

Pipeline::Pipeline(CameraCapture& cam, ColorTracker& tracker, OscSender& osc_sender)
    : cam_(cam), tracker_(tracker), osc_sender_(osc_sender) {}

// ============================================================
// start: 3 スレッドを起動する
// ============================================================
void Pipeline::start() {
    if (running_.load()) {
        return; // 多重起動防止
    }
    running_.store(true);

    // std::jthread のコンストラクタに渡す関数は
    // std::stop_token を第一引数に取ることができる（自動バインド）。
    // これが std::thread との大きな違い。
    capture_thread_ = std::jthread([this](std::stop_token token) {
        capture_loop(std::move(token));
    });

    detect_thread_ = std::jthread([this](std::stop_token token) {
        detect_loop(std::move(token));
    });

    send_thread_ = std::jthread([this](std::stop_token token) {
        send_loop(std::move(token));
    });

    std::cout << "[Pipeline] 起動完了（3スレッド）\n";
}

// ============================================================
// stop: 全スレッドに停止を要求して終了を待つ
// ============================================================
void Pipeline::stop() {
    if (!running_.load()) {
        return;
    }
    running_.store(false);

    // jthread::request_stop() → stop_token::stop_requested() が true になる
    // jthread のデストラクタが自動で request_stop() + join() を呼ぶが、
    // 明示的に呼ぶことでより意図が明確になる。
    capture_thread_.request_stop();
    detect_thread_.request_stop();
    send_thread_.request_stop();

    // jthread はデストラクタで join を呼ぶ（RAII）。
    // 明示的に join を呼ぶことで、全スレッドの終了を確認できる。
    if (capture_thread_.joinable()) capture_thread_.join();
    if (detect_thread_.joinable())  detect_thread_.join();
    if (send_thread_.joinable())    send_thread_.join();

    std::cout << "[Pipeline] 停止完了\n";
}

[[nodiscard]] bool Pipeline::is_running() const {
    return running_.load();
}

// ============================================================
// capture_loop: カメラスレッド
// ============================================================
void Pipeline::capture_loop(std::stop_token token) {
    while (!token.stop_requested()) {
        auto frame = cam_.read_frame();
        if (!frame.has_value()) {
            // フレーム取得失敗: 少し待ってリトライ
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        // キューが満杯なら古いフレームを捨てて新しいフレームを入れる。
        // リアルタイム処理では「最新フレームを優先」することが多い。
        if (!frame_queue_.try_push(*frame)) {
            // キュー満杯: このフレームはスキップ
        }
    }
}

// ============================================================
// detect_loop: 検出スレッド
// ============================================================
void Pipeline::detect_loop(std::stop_token token) {
    while (!token.stop_requested()) {
        auto frame = frame_queue_.try_pop();
        if (!frame.has_value()) {
            // キューが空: 少し待ってリトライ
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        auto result = tracker_.process(*frame);
        if (result.has_value()) {
            result_queue_.try_push(*result);
        }
    }
}

// ============================================================
// send_loop: OSC 送信スレッド
// ============================================================
void Pipeline::send_loop(std::stop_token token) {
    while (!token.stop_requested()) {
        auto result = result_queue_.try_pop();
        if (!result.has_value()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        // DetectorOutput の型に応じて OSC 送信
        std::visit([this](const auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, TrackResult>) {
                std::vector<OscValue> args{
                    val.centroid.x, val.centroid.y,
                    static_cast<float>(val.area),
                };
                osc_sender_.send("/tracker/centroid", args);
            } else if constexpr (std::is_same_v<T, FlowVec>) {
                std::vector<OscValue> args{
                    val.dx, val.dy, val.magnitude,
                };
                osc_sender_.send("/tracker/flow", args);
            }
        }, *result);
    }
}

} // namespace media
