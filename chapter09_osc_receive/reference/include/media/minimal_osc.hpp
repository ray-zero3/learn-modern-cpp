// ============================================================
// media/minimal_osc.hpp
// ------------------------------------------------------------
// OSC 1.0 準拠の最小実装（自前 UDP + バイト列生成）。
//
// なぜ oscpack を使わないのか:
//   oscpack の型定義 (int32 = signed long) は arm64 では 8 バイトになり、
//   "sizeof(osc::int32) == 4" のアサーションが失敗する（arm64 非対応）。
//   このプロジェクトは Apple Silicon (arm64) 環境なので、
//   最小自前実装で OSC 1.0 の UDP バイト列を生成する。
//
// OSC 1.0 のパケット構造（送信のみ実装）:
//   /address\0[padding]    : 4 バイトアライン
//   ,typetag\0[padding]    : 4 バイトアライン (例: ",iff\0")
//   [arg1][arg2]...        : 各引数 (big-endian, 4 または 8 バイト)
//
// 参照: https://opensoundcontrol.stanford.edu/spec-1_0.html
// ============================================================
#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// BSD ソケット API（macOS / Linux 共通）
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace media::osc {

// ============================================================
// Packet — OSC バイト列ビルダー
// ============================================================

namespace detail {

// 文字列を OSC フォーマット（null 終端 + 4 バイトパディング）に変換
inline std::vector<uint8_t> pad_string(std::string_view s) {
    std::vector<uint8_t> bytes(s.begin(), s.end());
    bytes.push_back('\0');
    // 4 バイトアラインになるまでゼロパディング
    while (bytes.size() % 4 != 0) {
        bytes.push_back('\0');
    }
    return bytes;
}

// ホストバイトオーダー → ビッグエンディアン (OSC は big-endian)
inline uint32_t to_be32(uint32_t val) {
    if constexpr (std::endian::native == std::endian::big) {
        return val;
    } else {
        return __builtin_bswap32(val);
    }
}

inline uint32_t float_to_be32(float val) {
    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(bits));
    return to_be32(bits);
}

} // namespace detail

// ============================================================
// 引数型 (osc_value.hpp の OscValue と連携)
// ============================================================

// OSC バイト列を組み立てる
class PacketBuilder {
public:
    PacketBuilder(std::string_view address, std::string_view type_tag) {
        auto addr_bytes = detail::pad_string(address);
        buffer_.insert(buffer_.end(), addr_bytes.begin(), addr_bytes.end());

        // type tag: カンマで始まる (例: ",iff")
        std::string tag = ",";
        tag += type_tag;
        auto tag_bytes = detail::pad_string(tag);
        buffer_.insert(buffer_.end(), tag_bytes.begin(), tag_bytes.end());
    }

    // int32 引数を追加
    PacketBuilder& add_int(int32_t val) {
        const uint32_t be = detail::to_be32(static_cast<uint32_t>(val));
        const auto* p = reinterpret_cast<const uint8_t*>(&be);
        buffer_.insert(buffer_.end(), p, p + 4);
        return *this;
    }

    // float32 引数を追加
    PacketBuilder& add_float(float val) {
        const uint32_t be = detail::float_to_be32(val);
        const auto* p = reinterpret_cast<const uint8_t*>(&be);
        buffer_.insert(buffer_.end(), p, p + 4);
        return *this;
    }

    // 文字列引数を追加
    PacketBuilder& add_string(std::string_view s) {
        auto bytes = detail::pad_string(s);
        buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());
        return *this;
    }

    // bool 引数を追加 (int32 で 0/1 に変換)
    PacketBuilder& add_bool(bool val) {
        return add_int(val ? 1 : 0);
    }

    [[nodiscard]] std::span<const uint8_t> data() const {
        return {buffer_.data(), buffer_.size()};
    }

private:
    std::vector<uint8_t> buffer_;
};

// ============================================================
// UdpSender — UDP でバイト列を送信
// ============================================================
class UdpSender {
public:
    UdpSender(std::string_view host, int port)
        : host_(host), port_(port) {
        // UDP ソケット作成
        sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ < 0) {
            connected_ = false;
            return;
        }

        // 送信先アドレス設定
        addr_.sin_family = AF_INET;
        addr_.sin_port   = htons(static_cast<uint16_t>(port));
        if (::inet_pton(AF_INET, std::string(host).c_str(), &addr_.sin_addr) <= 0) {
            ::close(sockfd_);
            sockfd_   = -1;
            connected_ = false;
            return;
        }
        connected_ = true;
    }

    ~UdpSender() {
        if (sockfd_ >= 0) {
            ::close(sockfd_);
        }
    }

    // コピー禁止、ムーブ可能
    UdpSender(const UdpSender&)            = delete;
    UdpSender& operator=(const UdpSender&) = delete;
    UdpSender(UdpSender&& other) noexcept
        : sockfd_(other.sockfd_), addr_(other.addr_),
          host_(std::move(other.host_)), port_(other.port_),
          connected_(other.connected_) {
        other.sockfd_    = -1;
        other.connected_ = false;
    }
    UdpSender& operator=(UdpSender&&) = delete;

    [[nodiscard]] bool is_connected() const { return connected_; }

    [[nodiscard]] bool send(std::span<const uint8_t> data) const {
        if (!connected_) return false;
        const ssize_t sent = ::sendto(sockfd_,
            data.data(), data.size(), 0,
            reinterpret_cast<const sockaddr*>(&addr_), sizeof(addr_));
        return sent == static_cast<ssize_t>(data.size());
    }

private:
    int sockfd_{-1};
    sockaddr_in addr_{};
    std::string host_;
    int port_;
    bool connected_{false};
};

} // namespace media::osc
