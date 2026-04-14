// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 07: 設定ファイル対応のデモアプリ。
//
// config.toml を読み込んでカメラID、HSV範囲、OSC設定を取得する。
// ============================================================
#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"
#include "media/config.hpp"
#include "media/osc_sender.hpp"
#include "media/osc_value.hpp"

#include <iostream>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, char** argv) {
    // -------------------------------------------------------
    // 設定ファイルの読み込み
    // -------------------------------------------------------
    const std::string config_path = (argc > 1) ? argv[1] : "config.toml";

    media::Config config;
    auto config_result = media::load_config(config_path);

    if (config_result.has_value()) {
        config = *config_result;
        std::cout << "[INFO] 設定ファイルを読み込みました: " << config_path << '\n';
    } else {
        // エラー時はデフォルト設定を使う（graceful degradation）
        std::cerr << "[警告] " << config_result.error().message << '\n';
        std::cerr << "[INFO] デフォルト設定を使用します。\n";
        config = media::make_default_config();
    }

    std::cout << "[INFO] カメラ: " << config.camera_id << '\n';
    std::cout << "[INFO] OSC: " << config.osc_host << ":" << config.osc_port << '\n';

    // -------------------------------------------------------
    // カメラ起動
    // -------------------------------------------------------
    media::CameraCapture cam;
    if (!cam.open(config.camera_id)) {
        std::cerr << "[エラー] カメラ " << config.camera_id << " を開けませんでした。\n";
        return 1;
    }

    media::ColorTracker tracker{config.hsv};
    media::OscSender osc_sender(config.osc_host, config.osc_port);

    std::cout << "[INFO] 'q' で終了。\n";

    while (true) {
        auto frame = cam.read_frame();
        if (!frame.has_value()) break;

        cv::Mat display = frame->clone();
        auto result = tracker.process(*frame);

        if (result.has_value()) {
            if (const auto* tr = std::get_if<media::TrackResult>(&result.value())) {
                cv::circle(display, tr->centroid, 20, cv::Scalar(0, 255, 0), 3);

                std::vector<media::OscValue> args{
                    tr->centroid.x, tr->centroid.y,
                    static_cast<float>(tr->area),
                };
                [[maybe_unused]] bool sent = osc_sender.send("/tracker/centroid", args);
            }
        }

        cv::imshow("Chapter 07 - Config", display);

        const int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 27) break;
    }

    cv::destroyAllWindows();
    return 0;
}
