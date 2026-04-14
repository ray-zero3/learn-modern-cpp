// ============================================================
// media/camera_capture.hpp (Chapter 02 継続)
// ------------------------------------------------------------
// Chapter 01 から引き続き使う RAII カメララッパ。
// Chapter 02 での変更なし。
// ============================================================
#pragma once

#include <optional>
#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>

namespace media {

class CameraCapture {
public:
    CameraCapture() = default;

    CameraCapture(const CameraCapture&)            = delete;
    CameraCapture& operator=(const CameraCapture&) = delete;

    CameraCapture(CameraCapture&&)            = default;
    CameraCapture& operator=(CameraCapture&&) = default;

    ~CameraCapture();

    [[nodiscard]] bool open(int device_id);
    [[nodiscard]] std::optional<cv::Mat> read_frame();
    [[nodiscard]] bool is_opened() const;

private:
    cv::VideoCapture cap_;
};

} // namespace media
