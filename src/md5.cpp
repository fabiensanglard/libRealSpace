#include "md5.h"
void MD5::init() {
    finalized = false;
    count[0] = count[1] = 0;
    // Load magic initialization constants.
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;
}

void MD5::update(const uint8_t* input, size_t length) {
    size_t index = (count[0] >> 3) & 0x3F;
    // Update number of bits.
    if ((count[0] += static_cast<uint32_t>(length << 3)) < (length << 3))
        count[1]++;
    count[1] += static_cast<uint32_t>(length >> 29);

    size_t partLen = 64 - index;
    size_t i = 0;

    if (length >= partLen) {
        memcpy(&buffer[index], input, partLen);
        transform(buffer);
        for (i = partLen; i + 63 < length; i += 64)
            transform(&input[i]);
        index = 0;
    }
    memcpy(&buffer[index], &input[i], length - i);
}

void MD5::finalize() {
    if (finalized) return;

    uint8_t bits[8];
    encode(bits, count, 8);

    // Pad out to 56 mod 64.
    size_t index = (count[0] >> 3) & 0x3F;
    size_t padLen = (index < 56) ? (56 - index) : (120 - index);
    static uint8_t PADDING[64] = { 0x80 };
    update(PADDING, padLen);
    update(bits, 8);
    encode(digest, state, 16);
    finalized = true;

    // Clean sensitive data.
    memset(buffer, 0, sizeof buffer);
    memset(count, 0, sizeof count);
    memset(state, 0, sizeof state);
}

std::string MD5::hexdigest() const {
    std::ostringstream oss;
    for (int i = 0; i < 16; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return oss.str();
}

void MD5::transform(const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
    decode(x, block, 64);

    // Round 1.
    a = b + rotate_left(a + F(b, c, d) + x[0]  + 0xd76aa478, S11);
    d = a + rotate_left(d + F(a, b, c) + x[1]  + 0xe8c7b756, S12);
    c = d + rotate_left(c + F(d, a, b) + x[2]  + 0x242070db, S13);
    b = c + rotate_left(b + F(c, d, a) + x[3]  + 0xc1bdceee, S14);

    a = b + rotate_left(a + F(b, c, d) + x[4]  + 0xf57c0faf, S11);
    d = a + rotate_left(d + F(a, b, c) + x[5]  + 0x4787c62a, S12);
    c = d + rotate_left(c + F(d, a, b) + x[6]  + 0xa8304613, S13);
    b = c + rotate_left(b + F(c, d, a) + x[7]  + 0xfd469501, S14);

    a = b + rotate_left(a + F(b, c, d) + x[8]  + 0x698098d8, S11);
    d = a + rotate_left(d + F(a, b, c) + x[9]  + 0x8b44f7af, S12);
    c = d + rotate_left(c + F(d, a, b) + x[10] + 0xffff5bb1, S13);
    b = c + rotate_left(b + F(c, d, a) + x[11] + 0x895cd7be, S14);

    a = b + rotate_left(a + F(b, c, d) + x[12] + 0x6b901122, S11);
    d = a + rotate_left(d + F(a, b, c) + x[13] + 0xfd987193, S12);
    c = d + rotate_left(c + F(d, a, b) + x[14] + 0xa679438e, S13);
    b = c + rotate_left(b + F(c, d, a) + x[15] + 0x49b40821, S14);

    // Round 2.
    a = b + rotate_left(a + G(b, c, d) + x[1]  + 0xf61e2562, S21);
    d = a + rotate_left(d + G(a, b, c) + x[6]  + 0xc040b340, S22);
    c = d + rotate_left(c + G(d, a, b) + x[11] + 0x265e5a51, S23);
    b = c + rotate_left(b + G(c, d, a) + x[0]  + 0xe9b6c7aa, S24);

    a = b + rotate_left(a + G(b, c, d) + x[5]  + 0xd62f105d, S21);
    d = a + rotate_left(d + G(a, b, c) + x[10] + 0x02441453, S22);
    c = d + rotate_left(c + G(d, a, b) + x[15] + 0xd8a1e681, S23);
    b = c + rotate_left(b + G(c, d, a) + x[4]  + 0xe7d3fbc8, S24);

    a = b + rotate_left(a + G(b, c, d) + x[9]  + 0x21e1cde6, S21);
    d = a + rotate_left(d + G(a, b, c) + x[14] + 0xc33707d6, S22);
    c = d + rotate_left(c + G(d, a, b) + x[3]  + 0xf4d50d87, S23);
    b = c + rotate_left(b + G(c, d, a) + x[8]  + 0x455a14ed, S24);

    a = b + rotate_left(a + G(b, c, d) + x[13] + 0xa9e3e905, S21);
    d = a + rotate_left(d + G(a, b, c) + x[2]  + 0xfcefa3f8, S22);
    c = d + rotate_left(c + G(d, a, b) + x[7]  + 0x676f02d9, S23);
    b = c + rotate_left(b + G(c, d, a) + x[12] + 0x8d2a4c8a, S24);

    // Round 3.
    a = b + rotate_left(a + H(b, c, d) + x[5]  + 0xfffa3942, S31);
    d = a + rotate_left(d + H(a, b, c) + x[8]  + 0x8771f681, S32);
    c = d + rotate_left(c + H(d, a, b) + x[11] + 0x6d9d6122, S33);
    b = c + rotate_left(b + H(c, d, a) + x[14] + 0xfde5380c, S34);

    a = b + rotate_left(a + H(b, c, d) + x[1]  + 0xa4beea44, S31);
    d = a + rotate_left(d + H(a, b, c) + x[4]  + 0x4bdecfa9, S32);
    c = d + rotate_left(c + H(d, a, b) + x[7]  + 0xf6bb4b60, S33);
    b = c + rotate_left(b + H(c, d, a) + x[10] + 0xbebfbc70, S34);

    a = b + rotate_left(a + H(b, c, d) + x[13] + 0x289b7ec6, S31);
    d = a + rotate_left(d + H(a, b, c) + x[0]  + 0xeaa127fa, S32);
    c = d + rotate_left(c + H(d, a, b) + x[3]  + 0xd4ef3085, S33);
    b = c + rotate_left(b + H(c, d, a) + x[6]  + 0x04881d05, S34);

    a = b + rotate_left(a + H(b, c, d) + x[9]  + 0xd9d4d039, S31);
    d = a + rotate_left(d + H(a, b, c) + x[12] + 0xe6db99e5, S32);
    c = d + rotate_left(c + H(d, a, b) + x[15] + 0x1fa27cf8, S33);
    b = c + rotate_left(b + H(c, d, a) + x[2]  + 0xc4ac5665, S34);

    // Round 4.
    a = b + rotate_left(a + I(b, c, d) + x[0]  + 0xf4292244, S41);
    d = a + rotate_left(d + I(a, b, c) + x[7]  + 0x432aff97, S42);
    c = d + rotate_left(c + I(d, a, b) + x[14] + 0xab9423a7, S43);
    b = c + rotate_left(b + I(c, d, a) + x[5]  + 0xfc93a039, S44);

    a = b + rotate_left(a + I(b, c, d) + x[12] + 0x655b59c3, S41);
    d = a + rotate_left(d + I(a, b, c) + x[3]  + 0x8f0ccc92, S42);
    c = d + rotate_left(c + I(d, a, b) + x[10] + 0xffeff47d, S43);
    b = c + rotate_left(b + I(c, d, a) + x[1]  + 0x85845dd1, S44);

    a = b + rotate_left(a + I(b, c, d) + x[8]  + 0x6fa87e4f, S41);
    d = a + rotate_left(d + I(a, b, c) + x[15] + 0xfe2ce6e0, S42);
    c = d + rotate_left(c + I(d, a, b) + x[6]  + 0xa3014314, S43);
    b = c + rotate_left(b + I(c, d, a) + x[13] + 0x4e0811a1, S44);

    a = b + rotate_left(a + I(b, c, d) + x[4]  + 0xf7537e82, S41);
    d = a + rotate_left(d + I(a, b, c) + x[11] + 0xbd3af235, S42);
    c = d + rotate_left(c + I(d, a, b) + x[2]  + 0x2ad7d2bb, S43);
    b = c + rotate_left(b + I(c, d, a) + x[9]  + 0xeb86d391, S44);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    // Zeroize sensitive information.
    memset(x, 0, sizeof(x));
}

// Computes and returns the MD5 hash as a hexadecimal string.
std::string ComputeMD5(const uint8_t* buffer, size_t length) {
    MD5 md5;
    md5.update(buffer, length);
    md5.finalize();
    return md5.hexdigest();
}
