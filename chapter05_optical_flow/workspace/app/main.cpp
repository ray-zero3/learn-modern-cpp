// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 04: Detector concept のデモアプリ。
//
// ColorTracker が Detector concept を満たすことを実際に使って確認する。
// テンプレート関数 run_detector<T> が Detector concept を要求する。
// ============================================================
#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"
#include "media/detector.hpp"
#include "media/osc_sender.hpp"
#include "media/osc_value.hpp"

#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// ============================================================
// run_detector: Detector concept を要求するテンプレート関数
// ------------------------------------------------------------
// T に Detector concept の制約を付けることで、
// process() メソッドを持たない型を渡すとコンパイルエラーになる。
//
// TS でいう:
//   function runDetector<T extends Detector>(detector: T, frame: Mat) { ... }
// に相当する。ただし C++ は実行時オーバーヘッドなし（静的ディスパッチ）。
// ============================================================
template <media::Detector T>
void run_detector(T& detector, const cv::Mat& frame, media::OscSender& osc) {
    auto result = detector.process(frame);
    if (!result.has_value()) {
        return;
    }

    // DetectorOutput の型に応じて OSC アドレスと引数を決定
    std::visit([&osc](const auto& val) {
        using OutputT = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<OutputT, media::TrackResult>) {
            std::vector<media::OscValue> args{
                val.centroid.x,
                val.centroid.y,
                static_cast<float>(val.area),
            };
            osc.send("/tracker/centroid", args);
        } else if constexpr (std::is_same_v<OutputT, media::FlowVec>) {
            std::vector<media::OscValue> args{
                val.dx, val.dy, val.magnitude,
            };
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

    // ColorTracker のインスタンス（青系の HSV 範囲）
    media::ColorTracker tracker{media::HsvRange{
        cv::Scalar(100, 80, 80),
        cv::Scalar(130, 255, 255)
    }};

    std::cout << "[INFO] Detector concept デモ起動。'q' で終了。\n";

    while (true) {
        auto frame = cam.read_frame();
        if (!frame.has_value()) break;

        // run_detector に ColorTracker を渡す。
        // コンパイラが ColorTracker::process() の存在と戻り値型を確認済み。
        run_detector(tracker, *frame, osc_sender);

        cv::imshow("Chapter 04 - Detector", *frame);

        const int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 27) break;
    }

    cv::destroyAllWindows();
    return 0;
}
