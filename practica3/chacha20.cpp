#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>

using State = std::array<uint32_t, 16>;

static inline uint32_t rotl32(uint32_t x, int r) {
  return (x << r) | (x >> (32 - r));
}

static inline uint32_t load32_le(const uint8_t b[4]) {
  return (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
         ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static inline void qr(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
  a += b; d ^= a; d = rotl32(d, 16);
  c += d; b ^= c; b = rotl32(b, 12);
  a += b; d ^= a; d = rotl32(d, 8);
  c += d; b ^= c; b = rotl32(b, 7);
}

static void print4x4(const State& s, const char* title) {
  std::cout << title << "\n";
  std::cout << std::hex << std::setfill('0') << std::nouppercase;
  for (int r = 0; r < 4; ++r) {
    std::cout << " ";
    for (int c = 0; c < 4; ++c) {
      std::cout << std::setw(8) << s[r * 4 + c] << (c == 3 ? "" : " ");
    }
    std::cout << "\n";
  }
  std::cout << std::dec;
}

static State build_state(const std::array<uint8_t,32>& key,
                         const std::array<uint8_t,4>& counter,
                         const std::array<uint8_t,12>& nonce) {
  State st{};
  st[0] = 0x61707865;
  st[1] = 0x3320646e;
  st[2] = 0x79622d32;
  st[3] = 0x6b206574;

  for (int i = 0; i < 8; ++i)
    st[4 + i] = load32_le(&key[i * 4]);

  st[12] = load32_le(counter.data());
  st[13] = load32_le(&nonce[0]);
  st[14] = load32_le(&nonce[4]);
  st[15] = load32_le(&nonce[8]);

  return st;
}

int main() {
  const std::array<uint8_t,32> key = {
    0x10,0x00,0x00,0x00,
    0x01,0x00,0x00,0x00,
    0x00,0x10,0x00,0x00,
    0x00,0x01,0x00,0x00,
    0x00,0x00,0x10,0x00,
    0x00,0x00,0x01,0x00,
    0x00,0x00,0x00,0x10,
    0x00,0x00,0x00,0x01
  };

  const std::array<uint8_t,4> counter = {0x01,0x00,0x00,0x00};

  const std::array<uint8_t,12> nonce = {
    0x00,0x00,0x00,0x09,
    0x00,0x00,0x00,0x4a,
    0x00,0x00,0x00,0x00
  };

  State st = build_state(key, counter, nonce);
  print4x4(st, "Estado inicial=");

  // 1ª diagonal de una diagonal-round: (0,5,10,15)
  qr(st[0], st[5], st[10], st[15]);

  print4x4(st, "Estado tras QR en 1ª diagonal (0,5,10,15)=");

  std::cout << std::hex << std::setfill('0') << std::nouppercase;
  std::cout << "Respuesta (state[0]) = " << std::setw(8) << st[0] << "\n";
  std::cout << std::dec;

  return 0;
}
