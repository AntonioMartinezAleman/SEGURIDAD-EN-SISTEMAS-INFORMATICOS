#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

std::string ToBinary(uint8_t value) {
  return std::bitset<8>(value).to_string();
}

uint8_t mulx(uint8_t value, uint8_t algorithm_byte) {
  if (value & 0x80) {
    return static_cast<uint8_t>((value << 1) ^ algorithm_byte);
  }
  return static_cast<uint8_t>(value << 1);
}

uint8_t multiply(uint8_t a, uint8_t b, uint8_t algorithm_byte) {
  uint8_t result = 0;
  uint8_t current = a;
  uint8_t mask = b;

  while (mask > 0) {
    if (mask & 0x01) {
      result ^= current;
    }
    current = mulx(current, algorithm_byte);
    mask >>= 1;
  }

  return result;
}

std::string Join(const std::vector<std::string>& parts, const std::string& separator) {
  std::string result;
  for (size_t i = 0; i < parts.size(); ++i) {
    result += parts[i];
    if (i + 1 < parts.size()) {
      result += separator;
    }
  }
  return result;
}

int main() {
  unsigned int first_byte;
  unsigned int second_byte;
  std::string algorithm;

  std::cout << "Primer byte (hex): ";
  std::cin >> std::hex >> first_byte;

  std::cout << "Segundo byte (hex): ";
  std::cin >> std::hex >> second_byte;

  std::cout << "Algoritmo (AES o SNOW3G): ";
  std::cin >> algorithm;

  uint8_t algorithm_byte;
  if (algorithm == "AES") {
    algorithm_byte = 0x1B;
  } else if (algorithm == "SNOW3G") {
    algorithm_byte = 0xA9;
  } else {
    std::cout << "Error: algoritmo no valido" << std::endl;
    return 1;
  }

  uint8_t a = static_cast<uint8_t>(first_byte);
  uint8_t b = static_cast<uint8_t>(second_byte);
  uint8_t result = multiply(a, b, algorithm_byte);

  std::vector<uint8_t> values(8);
  std::vector<std::string> step_descriptions(8);

  values[0] = a;
  step_descriptions[0] = ToBinary(a);

  for (int i = 1; i < 8; ++i) {
    uint8_t previous = values[i - 1];
    uint8_t shifted = static_cast<uint8_t>(previous << 1);
    uint8_t current = mulx(previous, algorithm_byte);
    values[i] = current;

    if (previous & 0x80) {
      step_descriptions[i] =
          ToBinary(shifted) + "+" + ToBinary(algorithm_byte) + "=" + ToBinary(current);
    } else {
      step_descriptions[i] = ToBinary(current);
    }
  }

  std::vector<std::string> expansion_masks;
  std::vector<std::string> expansion_values;

  for (int i = 0; i < 8; ++i) {
    if ((b >> i) & 0x01) {
      uint8_t mask_value = static_cast<uint8_t>(1u << i);
      expansion_masks.push_back(ToBinary(a) + "x" + ToBinary(mask_value));
      expansion_values.push_back(ToBinary(values[i]));
    }
  }

  std::cout << std::endl;
  std::cout << "Salida:" << std::endl;
  std::cout << std::endl;
  std::cout << "  - Primer byte: " << ToBinary(a) << std::endl;
  std::cout << "  - Segundo byte: " << ToBinary(b) << std::endl;
  std::cout << "  - Byte Algoritmo: " << ToBinary(algorithm_byte) << std::endl;
  std::cout << "  - Multiplicacion: " << ToBinary(result) << std::endl;

  std::cout << std::endl;
  std::cout << "Resultante de la operación:" << std::endl;
  std::cout << std::endl;

  std::cout << ToBinary(a) << "x" << ToBinary(b) << "="
            << Join(expansion_masks, " + ") << "=" << std::endl;

  std::cout << Join(expansion_values, "+") << "=" << ToBinary(result) << std::endl;

  std::cout << std::endl;
  return 0;
}