#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <queue>
#include <random>

using State = std::array<uint32_t, 16>;

static inline uint32_t rotl32(uint32_t x, int r) {
  return (x << r) | (x >> (32 - r));
}

static inline uint32_t load32_le(const uint8_t b[4]) {
  return (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
         ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static inline void qr(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
  uint32_t seed = 0;
  std::mt19937 rng(seed);
  std::array<int,4> base = {16,12,8,7};
  std::shuffle(base.begin(), base.end(), rng);

  std::queue<int> rotations;
  for (int r : base) rotations.push(r);

  a += b; d ^= a; d = rotl32(d, rotations.front()); rotations.pop();
  c += d; b ^= c; b = rotl32(b, rotations.front()); rotations.pop();
  a += b; d ^= a; d = rotl32(d, rotations.front()); rotations.pop();
  c += d; b ^= c; b = rotl32(b, rotations.front()); rotations.pop();
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

static State chacha20_block(const std::array<uint8_t,32>& key,
                            const std::array<uint8_t,4>& counter,
                            const std::array<uint8_t,12>& nonce,
                            State* initial_out,
                            State* after20_out) {

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

  if (initial_out) *initial_out = st;

  State x = st;

  for (int i = 0; i < 10; ++i) {

    qr(x[0], x[4], x[8],  x[12]);
    qr(x[1], x[5], x[9],  x[13]);
    qr(x[2], x[6], x[10], x[14]);
    qr(x[3], x[7], x[11], x[15]);

    qr(x[0], x[5], x[10], x[15]);
    qr(x[1], x[6], x[11], x[12]);
    qr(x[2], x[7], x[8],  x[13]);
    qr(x[3], x[4], x[9],  x[14]);
  }

  if (after20_out) *after20_out = x;

  State out{};
  for (int i = 0; i < 16; ++i)
    out[i] = x[i] + st[i];

  return out;
}

int main() {

  const std::array<uint8_t,32> key = {
    0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
    0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17,
    0x18,0x19,0x1a,0x1b, 0x1c,0x1d,0x1e,0x1f
  };

  const std::array<uint8_t,4> counter = {0x01,0x00,0x00,0x00};

  const std::array<uint8_t,12> nonce = {
    0x00,0x00,0x00,0x09,
    0x00,0x00,0x00,0x4a,
    0x00,0x00,0x00,0x00
  };

  State initial{}, after20{};
  State out = chacha20_block(key, counter, nonce, &initial, &after20);

  print4x4(initial, "Estado inicial=");
  print4x4(after20, "Estado final tras las 20 iteraciones=");
  print4x4(out,     "Estado de salida del generador=");

  return 0;
}