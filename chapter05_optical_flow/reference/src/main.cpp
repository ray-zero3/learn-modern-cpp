// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 05: オプティカルフロー + カラートラッカーのスイッチデモ。
//
// 's' キーで ColorTracker と OpticalFlowDetector を切り替える。
// どちらも Detector concept を満たすため、
// テンプレート関数 run_detector に渡せる。
// ============================================================
#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"
#include "media/optical_flow.hpp"
#include "media/detector.hpp"
#include "media/osc_sender.hpp"
#include "media/osc_value.hpp"

#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// Detector concept を要求するテンプレート関数
template <media::Detector T>
void run_detector(T& detector, const cv::Mat& frame,
                  media::OscSender& osc, cv::Mat& display) {
    auto result = detector.process(frame);
    if (!result.has_value()) return;

    std::visit([&osc, &display](const auto& val) {
        using OutputT = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<OutputT, media::TrackResult>) {
            // カラートラッカー結果: 重心に円を描画
            cv::circle(display, val.centroid, 20, cv::Scalar(0, 255, 0), 3);
            std::vector<media::OscValue> args{val.centroid.x, val.centroid.y,
                                               static_cast<float>(val.area)};
            osc.send("/tracker/centroid", args);

        } else if constexpr (std::is_same_v<OutputT, media::FlowVec>) {
            // フロー結果: 矢印で方向を描画
            const cv::Point center(display.cols / 2, display.rows / 2);
            const cv::Point tip(
                center.x + static_cast<int>(val.dx * 20),
                center.y + static_cast<int>(val.dy * 20)
            );
            cv::arrowedLine(display, center, tip, cv::Scalar(0, 0, 255), 3);

            const std::string info = "flow: " +
                std::to_string(static_cast<int>(val.magnitude * 100)) + "%";
            cv::putText(display, info, cv::Point(10, 30),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);

            std::vector<media::OscValue> args{val.dx, val.dy, val.magnitude};
            osc.send("/tracker/flow", args);
        }
    }, *result);
}

int main() {
    media::OscSender osc_sender("127.0.0.1", 9000);
    media::CameraCapture cam;

    if (!cam.open(0)) {
        std::cerr << "[エラー] カメラを開けませんでした。\n";
        return 1;
    }

    media::ColorTracker color_tracker{media::HsvRange{
        cv::Scalar(100, 80, 80),
        cv::Scalar(130, 255, 255)
    }};

    media::OpticalFlowDetector flow_detector;
    bool use_flow = false; // false = カラートラッカー, true = フロー

    std::cout << "[INFO] 's' キーでモード切替。'q' で終了。\n";
    std::cout << "[INFO] 現在: ColorTracker モード\n";

    while (true) {
        auto frame = cam.read_frame();
        if (!frame.has_value()) break;

        cv::Mat display = frame->clone();

        // モードに応じて検出器を切り替え
        if (use_flow) {
            run_detector(flow_detector, *frame, osc_sender, display);
        } else {
            run_detector(color_tracker, *frame, osc_sender, display);
        }

        // モード表示
        const std::string mode_text = use_flow ? "Optical Flow" : "Color Tracker";
        cv::putText(display, mode_text, cv::Point(10, display.rows - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);

        cv::imshow("Chapter 05 - Optical Flow", display);

        const int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 27) break;
        if (key == 's') {
            use_flow = !use_flow;
            flow_detector.reset(); // モード切替時にリセット
            std::cout << "[INFO] モード切替: " << mode_text << "\n";
        }
    }

    cv::destroyAllWindows();
    return 0;
}
