// ============================================================
// media/osc_receiver.hpp
// ------------------------------------------------------------
// OSC メッセージを UDP で受信するクラス。
//
// この章で学ぶ主要概念:
//   - 別スレッドでのUDP受信（バックグラウンドスレッド）
//   - コールバック登録パターン（Observer パターン）
//   - スレッドセーフなコールバック呼び出し
//
// TS 対比:
//   Node.js の dgram.createSocket('udp4').on('message', callback) に相当。
//   C++ では明示的なスレッド管理が必要。
// ============================================================
#pragma once

#include "media/osc_value.hpp"

#include <atomic>
#include <functional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace media {

// OSC メッセージのコールバック型
// address: /tracker/centroid のような OSC アドレス
// args: 引数リスト (OscValue の vector)
using OscMessageCallback = std::function<void(std::string_view address,
                                               const std::vector<OscValue>& args)>;

// ============================================================
// OscReceiver — UDP で OSC メッセージを受信するクラス
// ============================================================
class OscReceiver {
public:
    explicit OscReceiver(int port);
    ~OscReceiver();

    OscReceiver(const OscReceiver&)            = delete;
    OscReceiver& operator=(const OscReceiver&) = delete;
    OscReceiver(OscReceiver&&)                 = delete;
    OscReceiver& operator=(OscReceiver&&)      = delete;

    // コールバックを登録（複数登録可能）
    void on_message(OscMessageCallback callback);

    // 受信スレッドを開始
    void start();

    // 受信スレッドを停止
    void stop();

    [[nodiscard]] bool is_running() const;

private:
    int port_;
    int sockfd_{-1};
    std::atomic<bool> running_{false};
    std::jthread recv_thread_;
    std::vector<OscMessageCallback> callbacks_;

    void recv_loop(std::stop_token token);

    // 受信したバイト列を OSC メッセージとして解析
    void parse_and_dispatch(const char* buf, std::size_t len);
};

} // namespace media
