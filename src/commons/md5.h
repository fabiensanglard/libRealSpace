#include <cstdint>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

// Minimal MD5 implementation.
class MD5 {
public:
    MD5() { init(); }
    void update(const uint8_t* input, size_t length);
    void finalize();
    std::string hexdigest() const;

private:
    void init();
    void transform(const uint8_t block[64]);

    bool finalized;
    uint8_t buffer[64];    // input buffer
    uint32_t count[2];     // number of bits, modulo 2^64 (lsb first)
    uint32_t state[4];     // digest state
    uint8_t digest[16];    // final digest

    static inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
    static inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
    static inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
    static inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }
    static inline uint32_t rotate_left(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }
};

static void encode(uint8_t* output, const uint32_t* input, size_t len) {
    for (size_t i = 0, j = 0; j < len; i++, j += 4) {
        output[j]     = static_cast<uint8_t>( input[i]        & 0xff);
        output[j + 1] = static_cast<uint8_t>((input[i] >> 8)  & 0xff);
        output[j + 2] = static_cast<uint8_t>((input[i] >> 16) & 0xff);
        output[j + 3] = static_cast<uint8_t>((input[i] >> 24) & 0xff);
    }
}

static void decode(uint32_t* output, const uint8_t* input, size_t len) {
    for (size_t i = 0, j = 0; j < len; i++, j += 4)
        output[i] = static_cast<uint32_t>(input[j]) |
                    (static_cast<uint32_t>(input[j + 1]) << 8) |
                    (static_cast<uint32_t>(input[j + 2]) << 16) |
                    (static_cast<uint32_t>(input[j + 3]) << 24);
}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

std::string ComputeMD5(const uint8_t* buffer, size_t length);
// Example usage:
// int main() {
//     const uint8_t data[] = "The quick brown fox jumps over the lazy dog";
//     std::string hash = ComputeMD5(data, sizeof(data) - 1);
//     printf("MD5: %s\n", hash.c_str());
//     return 0;
// }