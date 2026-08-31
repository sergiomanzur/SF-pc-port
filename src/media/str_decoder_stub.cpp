#include "sf/media/str_decoder.hpp"

namespace sf::media {

struct StrDecoder::Impl {
    bool has_audio{false};
};

StrDecoder::StrDecoder(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
StrDecoder::~StrDecoder() = default;
StrDecoder::StrDecoder(StrDecoder&&) noexcept = default;
StrDecoder& StrDecoder::operator=(StrDecoder&&) noexcept = default;

StrDecoder StrDecoder::open(std::vector<std::byte>) {
    return StrDecoder{std::make_unique<Impl>()};
}

StrDecoder StrDecoder::open(std::span<const std::byte>) {
    return StrDecoder{std::make_unique<Impl>()};
}

std::optional<MovieEvent> StrDecoder::next() {
    return std::nullopt;
}

double StrDecoder::framesPerSecond() const noexcept {
    return 15.0;
}

bool StrDecoder::hasAudio() const noexcept {
    return false;
}

} // namespace sf::media
