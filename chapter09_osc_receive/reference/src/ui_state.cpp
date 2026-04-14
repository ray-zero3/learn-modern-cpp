// ============================================================
// src/ui_state.cpp
// ------------------------------------------------------------
// UiState の実装。
// ============================================================
#include "media/ui_state.hpp"

namespace media {

HsvRange UiState::get_hsv_range() const {
    std::lock_guard lock(mutex_);
    return hsv_range_;
}

void UiState::set_hsv_range(const HsvRange& range) {
    {
        std::lock_guard lock(mutex_);
        hsv_range_ = range;
    }
    notify_change();
}

float UiState::get_fps() const {
    return fps_.load();
}

void UiState::set_fps(float fps) {
    fps_.store(fps);
}

void UiState::add_log(const std::string& msg) {
    std::lock_guard lock(mutex_);
    logs_.push_back(msg);
    if (logs_.size() > kMaxLogs) {
        logs_.erase(logs_.begin());
    }
}

std::vector<std::string> UiState::get_logs() const {
    std::lock_guard lock(mutex_);
    return logs_;
}

void UiState::clear_logs() {
    std::lock_guard lock(mutex_);
    logs_.clear();
}

void UiState::on_change(ChangeCallback callback) {
    std::lock_guard lock(mutex_);
    change_callbacks_.push_back(std::move(callback));
}

void UiState::notify_change() {
    std::lock_guard lock(mutex_);
    for (const auto& cb : change_callbacks_) {
        cb();
    }
}

} // namespace media
