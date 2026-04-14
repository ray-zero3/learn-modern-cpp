// ============================================================
// src/osc_receiver.cpp
// ------------------------------------------------------------
// OscReceiver の実装。自前の OSC パーサーを使って
// 受信バイト列を解析し、コールバックを呼ぶ。
// ============================================================
#include "media/osc_receiver.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

namespace media {

// ============================================================
// コンストラクタ
// ============================================================
OscReceiver::OscReceiver(int port) : port_(port) {}

// ============================================================
// デストラクタ
// ============================================================
OscReceiver::~OscReceiver() {
    stop();
}

// ============================================================
// on_message: コールバックを登録
// ============================================================
void OscReceiver::on_message(OscMessageCallback callback) {
    callbacks_.push_back(std::move(callback));
}

// ============================================================
// start: 受信スレッドを開始
// ============================================================
void OscReceiver::start() {
    if (running_.load()) return;

    // UDP ソケット作成
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        std::cerr << "[OscReceiver] ソケット作成失敗\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port_));

    if (::bind(sockfd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[OscReceiver] bind 失敗: port=" << port_ << '\n';
        ::close(sockfd_);
        sockfd_ = -1;
        return;
    }

    // タイムアウト設定（stop_requested チェックのため）
    timeval tv{};
    tv.tv_sec  = 0;
    tv.tv_usec = 100000; // 100ms タイムアウト
    ::setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    running_.store(true);
    recv_thread_ = std::jthread([this](std::stop_token token) {
        recv_loop(std::move(token));
    });

    std::cout << "[OscReceiver] 受信開始: port=" << port_ << '\n';
}

// ============================================================
// stop: 受信スレッドを停止
// ============================================================
void OscReceiver::stop() {
    if (!running_.load()) return;
    running_.store(false);

    recv_thread_.request_stop();
    if (recv_thread_.joinable()) recv_thread_.join();

    if (sockfd_ >= 0) {
        ::close(sockfd_);
        sockfd_ = -1;
    }
}

// ============================================================
// is_running
// ============================================================
bool OscReceiver::is_running() const {
    return running_.load();
}

// ============================================================
// recv_loop: 受信ループ（別スレッドで実行）
// ============================================================
void OscReceiver::recv_loop(std::stop_token token) {
    constexpr std::size_t kBufSize = 4096;
    char buf[kBufSize];

    while (!token.stop_requested() && running_.load()) {
        const ssize_t received = ::recvfrom(sockfd_, buf, kBufSize, 0, nullptr, nullptr);
        if (received > 0) {
            parse_and_dispatch(buf, static_cast<std::size_t>(received));
        }
        // タイムアウト時は received <= 0 → ループ継続
    }
}

// ============================================================
// parse_and_dispatch: OSC パケットを解析してコールバックを呼ぶ
// ============================================================
// OSC 1.0 の最小パーサー（送信側と対称）
void OscReceiver::parse_and_dispatch(const char* buf, std::size_t len) {
    if (len < 4) return;

    // OSC アドレスを読む（null 終端 + 4 バイトアライン）
    std::string address;
    std::size_t pos = 0;
    while (pos < len && buf[pos] != '\0') {
        address += buf[pos];
        ++pos;
    }

    // アドレスの後の null + padding をスキップ
    while (pos < len && (pos % 4 != 0 || buf[pos] == '\0')) {
        ++pos;
    }

    // type tag を読む（カンマ始まり）
    if (pos >= len || buf[pos] != ',') return;
    ++pos; // カンマをスキップ

    std::string type_tag;
    while (pos < len && buf[pos] != '\0') {
        type_tag += buf[pos];
        ++pos;
    }

    // type tag の後の null + padding をスキップ
    while (pos < len && (pos % 4 != 0 || buf[pos] == '\0')) {
        ++pos;
    }

    // 引数を読む
    std::vector<OscValue> args;
    for (char tag : type_tag) {
        if (pos + 4 > len) break;

        if (tag == 'i') {
            // big-endian int32 を読む
            uint32_t be;
            std::memcpy(&be, buf + pos, 4);
            const int32_t val = static_cast<int32_t>(ntohl(be));
            args.push_back(static_cast<int>(val));
            pos += 4;

        } else if (tag == 'f') {
            // big-endian float32
            uint32_t be;
            std::memcpy(&be, buf + pos, 4);
            const uint32_t host = ntohl(be);
            float val;
            std::memcpy(&val, &host, 4);
            args.push_back(val);
            pos += 4;

        } else if (tag == 's') {
            // 文字列
            std::string s;
            while (pos < len && buf[pos] != '\0') {
                s += buf[pos];
                ++pos;
            }
            // null + padding スキップ
            while (pos < len && (pos % 4 != 0 || buf[pos] == '\0')) {
                ++pos;
            }
            args.push_back(s);
        }
    }

    // コールバックを呼ぶ
    for (const auto& cb : callbacks_) {
        cb(address, args);
    }
}

} // namespace media
