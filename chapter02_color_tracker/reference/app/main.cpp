// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 02: HSV 色域トラッカーのデモアプリ。
//
// カメラ映像から青い物体を追跡し、重心に円と面積を描画する。
// 'q' / ESC で終了。
//
// 実行方法:
//   ./build/chapter02_color_tracker/reference/chapter02_ref_app
// ============================================================
#include "media/camera_capture.hpp"
#include "media/color_tracker.hpp"

#include <iostream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main() {
    media::CameraCapture cam;
    if (!cam.open(0)) {
        std::cerr << "[エラー] カメラを開けませんでした。\n";
        return 1;
    }

    // 追跡する色域: 青系（HSV H=100-130）
    // 手元のオブジェクト（青いものを持ってカメラの前に出す）で確認できる。
    media::HsvRange blue_range{
        cv::Scalar(100, 80, 80),
        cv::Scalar(130, 255, 255)
    };

    std::cout << "[INFO] 青い物体を追跡します。'q' キーで終了。\n";

    while (true) {
        auto frame = cam.read_frame();
        if (!frame.has_value()) {
            std::cerr << "[警告] フレーム取得失敗。終了します。\n";
            break;
        }

        // 元フレームのコピーに描画する（元データは変えない）。
        // cv::Mat のコピーは参照カウントのコピー（内部バッファ共有）。
        // clone() で独立したバッファを作る。
        cv::Mat display = frame->clone();

        // トラッキング実行
        auto result = media::track(*frame, blue_range);

        if (result.has_value()) {
            // 重心に円を描画
            cv::circle(display,
                       result->centroid,      // 中心座標
                       20,                    // 半径
                       cv::Scalar(0, 255, 0), // 緑色
                       3);                    // 線の太さ

            // 面積テキストを表示
            const std::string area_text =
                "area: " + std::to_string(static_cast<int>(result->area));
            cv::putText(display, area_text,
                        cv::Point(10, 30),
                        cv::FONT_HERSHEY_SIMPLEX, 1.0,
                        cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Chapter 02 - Color Tracker", display);

        const int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 27) {
            break;
        }
    }

    cv::destroyAllWindows();
    return 0;
}
