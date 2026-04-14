// ============================================================
// app/main.cpp
// ------------------------------------------------------------
// Chapter 01: カメラ映像をウィンドウに表示するアプリ。
//
// CameraCapture (RAII ラッパ) を使って内蔵カメラからフレームを
// 取得し、cv::imshow でウィンドウに表示する。
// 'q' キーを押すか、ウィンドウを閉じると終了する。
//
// 実行方法:
//   ./build/chapter01_camera_view/reference/chapter01_ref_app
// ============================================================
#include "media/camera_capture.hpp"

#include <iostream>

// OpenCV の GUI / 描画関連ヘッダ
#include <opencv2/highgui.hpp>

int main() {
    // -------------------------------------------------------
    // カメラを開く
    // -------------------------------------------------------
    // RAII: このスコープを出たとき (return / 例外) に
    //   CameraCapture のデストラクタが自動的に cap_.release() を呼ぶ。
    //   TS でいう `using cam = ...` (Symbol.dispose) に似ているが、
    //   言語レベルで保証されている点が違う。
    media::CameraCapture cam;
    if (!cam.open(0)) {
        // open() 失敗時はエラーメッセージを出して終了。
        // 例外を使わず bool で失敗を伝える設計（Chapter 3 以降で
        // std::expected を使ったより型安全な設計に発展する）。
        std::cerr << "[エラー] カメラ (device 0) を開けませんでした。\n"
                  << "macOS の場合はシステム環境設定 → プライバシー → カメラ\n"
                  << "で権限が付与されているか確認してください。\n";
        return 1;
    }

    std::cout << "[INFO] カメラを開きました。'q' キーで終了します。\n";

    // -------------------------------------------------------
    // メインループ: フレームを読んで表示を繰り返す
    // -------------------------------------------------------
    while (true) {
        // read_frame() は std::optional<cv::Mat> を返す。
        // TS: const frame = cam.readFrame() の型は Mat | undefined 的な発想。
        auto frame = cam.read_frame();

        if (!frame.has_value()) {
            // フレーム取得失敗 (デバイスエラー or EOF)
            std::cerr << "[警告] フレーム取得に失敗しました。終了します。\n";
            break;
        }

        // ウィンドウに表示。ウィンドウ名が一致しない場合は新規作成される。
        cv::imshow("Chapter 01 - Camera View", *frame);

        // キー入力を 30ms 待つ。'q' で終了。
        // waitKey の戻り値は押されたキーの ASCII コード。
        // & 0xFF はプラットフォーム互換のためのマスク処理。
        const int key = cv::waitKey(30) & 0xFF;
        if (key == 'q' || key == 27 /* ESC */) {
            std::cout << "[INFO] 終了します。\n";
            break;
        }
    }

    // cam のデストラクタが自動的に cap_.release() を呼ぶ。
    // cv::destroyAllWindows() は必要に応じて呼ぶ（省略可）。
    cv::destroyAllWindows();
    return 0;
}
