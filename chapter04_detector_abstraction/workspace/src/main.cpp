// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 03: OSC 送信のデモアプリ。
//
// カメラ + HSV トラッカー + OSC 送信を統合する。
// 127.0.0.1:9000 に /tracker/centroid x y area を送信する。
//
// Max/MSP での受信確認: max_patches/ の受信パッチを使用。
//
// 実行方法:
//   ./build/chapter03_osc_sender/reference/chapter03_ref_app
// ============================================================
#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"
#include "media/osc_sender.hpp"
#include "media/osc_value.hpp"

#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    // OSC 送信先: ローカルの Max/MSP や TouchDesigner が待ち受けるポート
    constexpr auto kOscHost = "127.0.0.1";
    constexpr int  kOscPort = 9000;

    media::OscSender osc_sender(kOscHost, kOscPort);
    if (!osc_sender.is_connected()) {
        std::cerr << "[エラー] OSC ソケット作成失敗。\n";
        return 1;
    }
    std::cout << "[INFO] OSC 送信先: " << kOscHost << ":" << kOscPort << '\n';

    media::CameraCapture cam;
    if (!cam.open(0)) {
        std::cerr << "[エラー] カメラを開けませんでした。\n";
        return 1;
    }

    // 青系の HSV 範囲
    media::HsvRange blue_range{
        cv::Scalar(100, 80, 80),
        cv::Scalar(130, 255, 255)
    };

    std::cout << "[INFO] 青い物体を追跡して OSC 送信します。'q' で終了。\n";

    while (true) {
        auto frame = cam.read_frame();
        if (!frame.has_value()) {
            std::cerr << "[警告] フレーム取得失敗。\n";
            break;
        }

        cv::Mat display = frame->clone();
        auto result = media::track(*frame, blue_range);

        if (result.has_value()) {
            // OSC メッセージを組み立てて送信
            // アドレス: /tracker/centroid
            // 引数: x(float), y(float), area(float)
            std::vector<media::OscValue> args{
                result->centroid.x,
                result->centroid.y,
                static_cast<float>(result->area),
            };

            if (!osc_sender.send("/tracker/centroid", args)) {
                std::cerr << "[警告] OSC 送信失敗。\n";
            }

            // 画面に描画
            cv::circle(display, result->centroid, 20, cv::Scalar(0, 255, 0), 3);

            const std::string info_text =
                "x:" + std::to_string(static_cast<int>(result->centroid.x)) +
                " y:" + std::to_string(static_cast<int>(result->centroid.y)) +
                " area:" + std::to_string(static_cast<int>(result->area));
            cv::putText(display, info_text,
                        cv::Point(10, 30),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7,
                        cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Chapter 03 - OSC Sender", display);

        const int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 27) {
            break;
        }
    }

    cv::destroyAllWindows();
    return 0;
}
