#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>

std::string vigenere(std::string& texto, std::string& clave) {
  for (char& c : texto) c = std::toupper(static_cast<unsigned char>(c));
  for (char& c : clave) c = std::toupper(static_cast<unsigned char>(c));

  std::string cifrado;
  cifrado.resize(texto.size());

  int contador = 0;
  for (int i = 0; i < (int)texto.size(); i++) {
    if (texto[i] >= 'A' && texto[i] <= 'Z') {
      int t = texto[i] - 'A';
      int k = clave[contador % (int)clave.size()] - 'A';
      cifrado[i] = char((t + k) % 26 + 'A');
      contador++;
    } else {
      cifrado[i] = texto[i];
    }
  }
  return cifrado;
}

std::string descifrar(std::string texto, std::string clave) {
  for (char& c : texto) c = std::toupper(static_cast<unsigned char>(c));
  for (char& c : clave) c = std::toupper(static_cast<unsigned char>(c));

  std::string claro;
  claro.resize(texto.size());

  int contador = 0;
  for (int i = 0; i < (int)texto.size(); i++) {
    if (texto[i] >= 'A' && texto[i] <= 'Z') {
      int t = texto[i] - 'A';
      int k = clave[contador % (int)clave.size()] - 'A';

      claro[i] = char((t - k + 26) % 26 + 'A');  // resta clave
      contador++;
    } else {
      claro[i] = texto[i];
    }
  }
  return claro;
}

std::string agrupar(const std::string& s, int n) {
  std::string out;
  int cnt = 0;
  for (char c : s) {
    if (c == ' ') continue;
    if (cnt > 0 && cnt % n == 0) out.push_back(' ');
    out.push_back(c);
    cnt++;
  }
  return out;
}

int main() {
  std::string texto = "XKDILSMXAG";
  std::string clave = "SOL";

  std::string cifrado = vigenere(texto, clave);

  std::cout << "Cifrado:\n";
  std::cout << agrupar(cifrado, (int)clave.size()) << "\n\n";

  std::string descifrado = descifrar(cifrado, clave);

  std::cout << "Descifrado:\n";
  std::cout << descifrado << "\n";

  return 0;
}