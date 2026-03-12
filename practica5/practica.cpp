#include <iostream>
#include <iomanip>
#include <bitset>
#include <string>
#include <cstdint>

/**
 * @brief Multiplica un polinomio por x en el cuerpo finito GF(2^8).
 *
 * En GF(2^8) cada byte representa un polinomio de grado <= 7. Por ejemplo,
 * 0x57 = 01010111 representa x^6 + x^4 + x^2 + x + 1.
 *
 * Multiplicar por x equivale a desplazar los coeficientes una posición
 * (shift left), lo que aumenta el grado del polinomio en 1.
 *
 * Si el bit más significativo (coeficiente de x^7) es 1, el resultado
 * tendría grado 8 y se saldría del cuerpo GF(2^8). Para mantenerlo
 * dentro del cuerpo se aplica una reducción modular: XOR con el
 * polinomio irreducible del algoritmo:
 *   - AES:    0x1B → x^8 + x^4 + x^3 + x + 1
 *   - SNOW3G: 0xA9 → x^8 + x^7 + x^5 + x^3 + 1
 * (Los bytes 0x1B y 0xA9 son los 8 bits bajos del polinomio irreducible,
 *  ya que el término x^8 se cancela implícitamente con el bit desbordado.)
 *
 * @param value Byte (polinomio) que se va a multiplicar por x.
 * @param algorithm_byte Bits bajos del polinomio irreducible (0x1B para AES, 0xA9 para SNOW3G).
 * @return uint8_t Resultado de value · x (mod polinomio irreducible).
 */
uint8_t mulx(uint8_t value, uint8_t algorithm_byte) {
  if (value & 0x80) { // Si el bit más significativo es 1 antes del desplazamiento
    return static_cast<uint8_t>((value << 1) ^ algorithm_byte);
  }
  return static_cast<uint8_t>(value << 1);
}

/**
 * @brief Multiplica dos polinomios en GF(2^8) usando el método "shift-and-add".
 *
 * El algoritmo es análogo a la multiplicación larga en binario, pero
 * adaptado a GF(2^8): la suma se sustituye por XOR y la multiplicación
 * por x se realiza con mulx() (que incluye reducción modular).
 *
 * Sea b = b7·x^7 + b6·x^6 + ... + b1·x + b0. Entonces:
 *   a · b = a·b0 + a·x·b1 + a·x^2·b2 + ... + a·x^7·b7
 *
 * El bucle recorre los bits de b de menos a más significativo:
 *   1. Si el bit actual de b es 1 → se acumula (XOR) el valor actual de a
 *      en el resultado (equivale a sumar ese término del desarrollo).
 *   2. Se multiplica a por x (mulx) para preparar el siguiente término.
 *   3. Se desplaza b a la derecha para examinar el siguiente bit.
 *
 * Al final, result contiene a · b (mod polinomio irreducible).
 *
 * @param a Primer byte (polinomio multiplicando).
 * @param b Segundo byte (polinomio multiplicador, se recorre bit a bit).
 * @param algorithm_byte Bits bajos del polinomio irreducible del algoritmo.
 * @return uint8_t Resultado de a · b en GF(2^8).
 */
uint8_t multiply(uint8_t a, uint8_t b, uint8_t algorithm_byte) {
  uint8_t result = 0;
  uint8_t current = a;
  uint8_t mask = b;

  while (mask > 0) {
    if (mask & 0x01) {
      result ^= current; // Si el bit menos significativo de b es 1, se acumula el valor actual
    }

    current = mulx(current, algorithm_byte);
    mask >>= 1;
  }

  return result;
}

/**
 * @brief Programa principal que calcula la multiplicación en GF(2^8).
 *
 * Solicita al usuario dos bytes en hexadecimal (cada uno representa un
 * polinomio de grado <= 7 en GF(2^8)) y el algoritmo criptográfico a
 * utilizar, que determina el polinomio irreducible para la reducción:
 *   - AES:    p(x) = x^8 + x^4 + x^3 + x + 1       (byte de reducción: 0x1B)
 *   - SNOW3G: p(x) = x^8 + x^7 + x^5 + x^3 + 1     (byte de reducción: 0xA9)
 *
 * Calcula el producto de ambos polinomios en GF(2^8) y muestra los
 * operandos y el resultado en representación binaria (8 bits = 8 coeficientes
 * del polinomio, de x^7 a x^0).
 *
 * @return int Código de salida del programa (0 si éxito, 1 si algoritmo inválido).
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

  std::cout << std::endl;
  std::cout << "Primer byte:     " << std::bitset<8>(a) << std::endl;
  std::cout << "Segundo byte:    " << std::bitset<8>(b) << std::endl;
  std::cout << "Byte Algoritmo:  " << std::bitset<8>(algorithm_byte) << std::endl;
  std::cout << "Multiplicacion:  " << std::bitset<8>(result) << std::endl;

  return 0;
}