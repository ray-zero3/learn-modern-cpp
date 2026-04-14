// ============================================================
// src/camera_capture.cpp
// ------------------------------------------------------------
// CameraCapture の実装。ヘッダの「宣言」に対する「定義」。
//
// ポイント:
//   - デストラクタを明示的に実装することで「RAII の契約」を満たす
//   - open() の失敗パスを明示的に扱い、例外を使わない設計
// ============================================================
#include "media/camera_capture.hpp"

namespace media {

// -------------------------------------------------------
// デストラクタ: カメラリソースを解放
// -------------------------------------------------------
// cv::VideoCapture のデフォルトデストラクタでも release() は
// 呼ばれるが、明示的に書くことで「ここで解放する」意図を
// コードに残す（チームや学習者への情報）。
CameraCapture::~CameraCapture() {
    if (cap_.isOpened()) {
        cap_.release();
    }
}

// -------------------------------------------------------
// open: カメラを開く
// -------------------------------------------------------
bool CameraCapture::open(int device_id) {
    // すでに開いている場合は先に閉じる（再オープン対応）
    if (cap_.isOpened()) {
        cap_.release();
    }

    // cv::VideoCapture::open() は成功/失敗を bool で返す。
    // 失敗時は例外ではなく false を返す設計（OpenCV の流儀）。
    return cap_.open(device_id);
}

// -------------------------------------------------------
// read_frame: フレームを1枚読む
// -------------------------------------------------------
std::optional<cv::Mat> CameraCapture::read_frame() {
    // カメラが開いていない場合は即座に nullopt を返す。
    // nullptr チェックと同じ発想だが、型で「値がないこと」を表現。
    if (!cap_.isOpened()) {
        return std::nullopt;
    }

    cv::Mat frame;
    // cap_ >> frame は cap_.read(frame) のシュガー。
    // 失敗（EOFやデバイスエラー）時は frame.empty() == true になる。
    cap_ >> frame;

    if (frame.empty()) {
        return std::nullopt;
    }

    // std::optional<cv::Mat> を返す。
    // cv::Mat の内部は参照カウント付きバッファなので、
    // ここで「コピー」しても実際にはポインタのコピーのみ（O(1)）。
    // 深掘り: docs/deepdive/02-move-semantics.md 参照
    return frame;
}

// -------------------------------------------------------
// is_opened: カメラの開閉状態を返す
// -------------------------------------------------------
bool CameraCapture::is_opened() const {
    return cap_.isOpened();
}

} // namespace media
