#include <iostream>
#include <vector>
#include <string>
#include <random>

std::vector<int> GenerarClaveAleatoria(size_t n_bits) {
  std::vector<int> clave;
  clave.reserve(n_bits);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, 1);

  for (size_t i = 0; i < n_bits; i++) {
    clave.push_back(dist(gen));
  }
  return clave;
}

std::vector<int> Vernam(const std::vector<int>& mensaje, const std::vector<int>& clave) {
  std::vector<int> cifrado;
  cifrado.reserve(mensaje.size());
  for (size_t i = 0; i < mensaje.size(); i++) {
    cifrado.push_back(mensaje[i] ^ clave[i]);
  }
  return cifrado;
}

std::vector<int> LetraToAscii(char c) {
  std::vector<int> bits(8);
  unsigned char v = static_cast<unsigned char>(c);
  for (int i = 7; i >= 0; i--) {
    bits[7 - i] = (v >> i) & 1;
  }
  return bits;
}

std::vector<int> StringToVectorBinario(const std::string& s) {
  std::vector<int> out;
  out.reserve(s.size() * 8);
  for (char c : s) {
    auto b = LetraToAscii(c);
    out.insert(out.end(), b.begin(), b.end());
  }
  return out;
}

void PrintVector(const std::vector<int>& v) {
  for (int bit : v) std::cout << bit;
  std::cout << "\n";
}

void PrintMessage(const std::vector<int>& vector_binario) {
  for (size_t i = 0; i + 7 < vector_binario.size(); i += 8) {
    int value = 0;
    for (int j = 0; j < 8; j++) {
      value = (value << 1) | vector_binario[i + j];
    }
    std::cout << static_cast<char>(value);
  }
  std::cout << "\n";
}


std::vector<int> SVectorToVector(const std::string& s) {
  std::vector<int> v;
  for (char c : s){
    v.push_back(c - '0');
  }
  return v;
}

int main() {
  std::string mensaje;
  std::cout << "Mensaje original:\n";
  std::cin >> mensaje;

  std::vector<int> mensaje_binario = StringToVectorBinario(mensaje);
  std::cout << "Mensaje original en binario: ";
  PrintVector(mensaje_binario);
  std::cout << "Longitud del mensaje binario: " << mensaje_binario.size() << "\n";

  std::cout << "Clave introducida: ";
  std::string clave_en_string;
  std::cin >> clave_en_string;
  std::vector<int> clave = SVectorToVector(clave_en_string);
  PrintVector(clave);

  std::vector<int> cifrado_bin = Vernam(mensaje_binario, clave);
  std::cout << "Mensaje cifrado en binario: ";
  PrintVector(cifrado_bin);

  std::cout << "Mensaje cifrado: ";
  PrintMessage(cifrado_bin);

  return 0;
}