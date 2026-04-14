// ============================================================
// media/ui_state.hpp
// ------------------------------------------------------------
// GUI で表示・編集するパラメータ状態。
// スレッドセーフ（複数スレッドから安全にアクセス可能）。
//
// この章で学ぶ主要概念:
//   - std::mutex + std::lock_guard でスレッドセーフなアクセス
//   - Observer パターン（変更通知コールバック）
//   - GUI ステートとビジネスロジックの分離
// ============================================================
#pragma once

#include "media/color_tracker.hpp" // HsvRange

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace media {

// ============================================================
// UiState — GUI パラメータの状態
// ============================================================
class UiState {
public:
    UiState() = default;

    // HSV 範囲の取得/設定（スレッドセーフ）
    [[nodiscard]] HsvRange get_hsv_range() const;
    void set_hsv_range(const HsvRange& range);

    // FPS の取得/設定
    [[nodiscard]] float get_fps() const;
    void set_fps(float fps);

    // OSC ログメッセージの追加/取得
    void add_log(const std::string& msg);
    [[nodiscard]] std::vector<std::string> get_logs() const;
    void clear_logs();

    // 変更通知コールバックの登録
    using ChangeCallback = std::function<void()>;
    void on_change(ChangeCallback callback);

private:
    // データメンバ
    mutable std::mutex mutex_;

    HsvRange hsv_range_{
        cv::Scalar(100, 80, 80),
        cv::Scalar(130, 255, 255)
    };

    std::atomic<float> fps_{0.0F};

    std::vector<std::string> logs_;
    static constexpr std::size_t kMaxLogs = 100;

    std::vector<ChangeCallback> change_callbacks_;

    void notify_change();
};

} // namespace media
