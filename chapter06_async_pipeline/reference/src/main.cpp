// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 06: 非同期パイプラインのデモアプリ。
//
// Capture / Detect / Send を別スレッドで並行処理する。
// ============================================================
#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"
#include "media/osc_sender.hpp"
#include "media/pipeline.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include <opencv2/highgui.hpp>

int main() {
    media::CameraCapture cam;
    if (!cam.open(0)) {
        std::cerr << "[エラー] カメラを開けませんでした。\n";
        return 1;
    }

    media::ColorTracker tracker{media::HsvRange{
        cv::Scalar(100, 80, 80),
        cv::Scalar(130, 255, 255)
    }};

    media::OscSender osc_sender("127.0.0.1", 9000);

    // パイプラインを起動
    media::Pipeline pipeline(cam, tracker, osc_sender);
    pipeline.start();

    std::cout << "[INFO] 非同期パイプライン起動。'q' で終了（コンソールに入力）。\n";
    std::cout << "[INFO] GUI 表示なし（パイプラインはバックグラウンドで動作）\n";

    // 10 秒間動作させてから終了
    std::this_thread::sleep_for(std::chrono::seconds(10));

    pipeline.stop();
    std::cout << "[INFO] 正常終了\n";
    return 0;
}
