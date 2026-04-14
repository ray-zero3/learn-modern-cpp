// ============================================================
// media/osc_sender.hpp
// ------------------------------------------------------------
// OSC メッセージを UDP で送信するクラスの宣言。
//
// oscpack ライブラリを内部で使用する。
// 外部インターフェースは oscpack に依存しないように設計し、
// 実装詳細を隠蔽する（PIMPL 的な発想）。
//
// TS 対比:
//   new UdpSender(host, port).send(address, args) に相当するが、
//   RAII により close() の明示呼び出しが不要。
// ============================================================
#pragma once

#include "media/osc_value.hpp"

#include <span>
#include <string>
#include <string_view>

namespace media {

// ============================================================
// OscSender — OSC メッセージ送信クラス
// ------------------------------------------------------------
// RAII で UDP ソケットを管理する。
// デストラクタでソケットを閉じる。
// ============================================================
class OscSender {
public:
    // コンストラクタ: 送信先ホストとポートを指定
    // TS: new OscSender("127.0.0.1", 9000)
    OscSender(std::string_view host, int port);

    // RAII: デストラクタでソケットを解放
    ~OscSender();

    // コピー禁止 (ソケットは1つ)
    OscSender(const OscSender&)            = delete;
    OscSender& operator=(const OscSender&) = delete;

    // ムーブ可能
    OscSender(OscSender&&)            = default;
    OscSender& operator=(OscSender&&) = default;

    // -------------------------------------------------------
    // send: OSC メッセージを送信
    // -------------------------------------------------------
    // address: OSC アドレスパターン (例: "/tracker/centroid")
    // args   : 引数のスパン (std::span は非所有ビュー)
    //
    // std::span<const OscValue> は:
    //   - TS の ReadonlyArray<OscValue> に近い
    //   - 配列/vector/initializer_list を統一的に受け取れる
    //   - コピーを行わない非所有ビュー
    //
    // [[nodiscard]]: 送信に失敗したとき false を返す。無視したらエラー。
    [[nodiscard]] bool send(std::string_view address,
                             std::span<const OscValue> args);

    // 接続状態の確認
    [[nodiscard]] bool is_connected() const;

private:
    std::string host_;
    [[maybe_unused]] int port_; // 接続先ポート番号（デバッグ用に保持）
    bool connected_{false};

    // oscpack の UdpTransmitSocket を保持する。
    // ポインタで保持して oscpack ヘッダの依存をソースファイルに閉じ込める
    // （= PIMPL パターンの簡易版）。
    // TS でいう private な実装詳細の型。
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace media
