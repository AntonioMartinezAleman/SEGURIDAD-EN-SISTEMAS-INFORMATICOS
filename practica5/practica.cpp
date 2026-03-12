#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Convierte un byte a una cadena binaria de 8 bits.
 * @param value Byte a convertir.
 * @return std::string Representación binaria.
 */
std::string ToBinary(uint8_t value) {
  return std::bitset<8>(value).to_string();
}

/**
 * @brief Multiplica un polinomio por x en GF(2^8).
 *
 * Si el bit más significativo es 1, tras desplazar a la izquierda
 * se aplica reducción con XOR usando el byte del algoritmo.
 *
 * @param value Byte actual.
 * @param algorithm_byte Byte de reducción (AES: 0x1B, SNOW3G: 0xA9).
 * @return uint8_t Resultado tras multiplicar por x.
 */
uint8_t mulx(uint8_t value, uint8_t algorithm_byte) {
  if (value & 0x80) {
    return static_cast<uint8_t>((value << 1) ^ algorithm_byte);
  }
  return static_cast<uint8_t>(value << 1);
}

/**
 * @brief Multiplica dos bytes en GF(2^8).
 *
 * Recorre el multiplicador bit a bit, acumulando con XOR
 * los términos correspondientes.
 *
 * @param a Primer byte.
 * @param b Segundo byte.
 * @param algorithm_byte Byte de reducción del algoritmo.
 * @return uint8_t Resultado de la multiplicación.
 */
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

/**
 * @brief Une una lista de cadenas con un separador.
 * @param parts Lista de cadenas.
 * @param separator Separador.
 * @return std::string Cadena final unida.
 */
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

/**
 * @brief Programa principal.
 * @return int 0 si todo fue bien, 1 si el algoritmo no es válido.
 */
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
  std::cout << "Donde la operación " << ToBinary(a) << "x10000000 resulta de los pasos:" << std::endl;
  std::cout << std::endl;

  for (int i = 0; i < 8; ++i) {
    std::cout << std::setw(22) << i;
  }
  std::cout << std::endl;

  for (int i = 0; i < 8; ++i) {
    std::cout << std::setw(22) << step_descriptions[i];
  }
  std::cout << std::endl;

  return 0;
}