// ============================================================
// tests/test_camera_capture.cpp
// ------------------------------------------------------------
// CameraCapture の単体テスト。
// 実カメラを開かない範囲でテストする設計。
//
// テスト設計の方針:
//   - open(-1) : 絶対に失敗するデバイスIDで false を返すことを確認
//   - 未 open 状態の状態テスト
//   - ムーブ後の元オブジェクトの安全性確認
//
// doctest の規約:
//   DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN を定義したファイルは
//   1テストターゲットに1つだけ存在できる（main が二重定義になるため）。
// ============================================================
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "media/camera_capture.hpp"

TEST_CASE("CameraCapture: デフォルト構築後は未オープン状態") {
    media::CameraCapture cam;
    // デフォルトコンストラクタ後、カメラは開いていない。
    CHECK(cam.is_opened() == false);
}

TEST_CASE("CameraCapture: デフォルト構築後は read_frame が nullopt を返す") {
    media::CameraCapture cam;
    // 未オープン状態での read_frame は std::nullopt であること。
    // std::optional の has_value() == false で確認できる。
    auto frame = cam.read_frame();
    CHECK(frame.has_value() == false);
}

TEST_CASE("CameraCapture: 無効なデバイスIDで open が false を返す") {
    media::CameraCapture cam;
    // -1 は存在しないデバイスID。どの環境でも失敗するはず。
    // [[nodiscard]] があるので戻り値を受け取る必要がある。
    bool result = cam.open(-1);
    CHECK(result == false);
    // open 失敗後も is_opened() は false のまま。
    CHECK(cam.is_opened() == false);
}

TEST_CASE("CameraCapture: open 失敗後の read_frame は nullopt") {
    media::CameraCapture cam;
    // open の戻り値は [[nodiscard]] なので必ず受け取る。
    // この変数は失敗確認に使わないが、意図を示すために保持する。
    [[maybe_unused]] bool ok = cam.open(-1); // 失敗することが前提
    auto frame = cam.read_frame();
    CHECK(frame.has_value() == false);
}

TEST_CASE("CameraCapture: ムーブ後の元オブジェクトは安全 (is_opened == false)") {
    media::CameraCapture cam_a;
    // ムーブ構築: cam_a の所有権を cam_b に移す。
    // TS のイメージ: 参照の付け替えではなく「所有権ごと移動」。
    // ムーブ後の cam_a は「有効だが未定義ではない」状態 = is_opened() == false
    media::CameraCapture cam_b = std::move(cam_a);

    // ムーブ元 cam_a は is_opened() == false であること（安全なアクセス可能）。
    // cv::VideoCapture のムーブ後は isOpened() == false が保証されている。
    CHECK(cam_a.is_opened() == false); // NOLINT(bugprone-use-after-move)

    // cam_b は cam_a の状態を引き継いでいる（元が未開放なので未開放）。
    CHECK(cam_b.is_opened() == false);
}

TEST_CASE("CameraCapture: ムーブ代入後の元オブジェクトは安全") {
    media::CameraCapture cam_a;
    media::CameraCapture cam_b;

    // ムーブ代入
    cam_b = std::move(cam_a);

    // ムーブ元は安全な状態（is_opened == false）であること。
    CHECK(cam_a.is_opened() == false); // NOLINT(bugprone-use-after-move)
    CHECK(cam_b.is_opened() == false);
}
