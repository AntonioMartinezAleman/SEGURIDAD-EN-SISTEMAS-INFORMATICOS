/**
 * @file aes.cpp
 * @brief Implementación del cifrado AES-128 (Advanced Encryption Standard).
 *
 * Este programa cifra un bloque de 16 bytes usando una clave de 16 bytes
 * siguiendo el estándar AES-128 con 10 rondas de cifrado.
 * La entrada y salida se manejan en formato hexadecimal.
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>

using namespace std;

/**
 * @brief Tabla de sustitución (S-Box) del estándar AES.
 *
 * Tabla de 256 valores predefinida por el estándar. Dado un byte de entrada
 * (0x00 a 0xFF), devuelve otro byte como sustituto. Se usa en SubBytes
 * y SubWord para introducir confusión en el cifrado.
 */
const unsigned char SBOX[256] = {
  0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
  0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
  0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
  0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
  0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
  0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
  0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
  0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
  0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
  0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
  0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
  0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
  0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
  0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
  0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
  0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

/**
 * @brief Constantes de ronda (Round Constants) del estándar AES.
 *
 * Se usan durante la expansión de clave. Hay una constante por cada ronda
 * (10 en total para AES-128). Se aplican mediante XOR al generar nuevas
 * palabras de la clave expandida.
 */
const unsigned char RCON[10] = {
  0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
};

/**
 * @brief Valida que una cadena sea hexadecimal de exactamente 32 caracteres.
 *
 * Comprueba que la entrada del usuario tenga el formato correcto:
 * 32 caracteres hexadecimales (0-9, a-f, A-F), lo que equivale a 16 bytes.
 *
 * @param s Cadena a validar.
 * @return true si es una cadena hexadecimal válida de 32 caracteres, false en caso contrario.
 */
bool esHex(const string& s) {
  if (s.size() != 32) return false;
  for (size_t i = 0; i < s.size(); i++) {
    if (!isxdigit(static_cast<unsigned char>(s[i]))) return false;
  }
  return true;
}

/**
 * @brief Convierte una cadena hexadecimal en un vector de bytes.
 *
 * Toma los caracteres de dos en dos y convierte cada par a su valor
 * numérico. Por ejemplo, "4f6e" se convierte en {0x4F, 0x6E}.
 *
 * @param s Cadena hexadecimal de entrada.
 * @return Vector de bytes resultante.
 */
vector<unsigned char> hexABytes(const string& s) {
  vector<unsigned char> bytes;
  for (size_t i = 0; i < s.size(); i += 2) {
    string par = s.substr(i, 2);
    unsigned int x;
    stringstream ss;
    ss << hex << par;
    ss >> x;
    bytes.push_back(static_cast<unsigned char>(x));
  }
  return bytes;
}

/**
 * @brief Convierte un vector de bytes en una cadena hexadecimal.
 *
 * Operación inversa a hexABytes. Cada byte se representa con dos
 * dígitos hexadecimales en minúscula. Por ejemplo, {0x4F, 0x6E}
 * se convierte en "4f6e".
 *
 * @param v Vector de bytes a convertir.
 * @return Cadena hexadecimal resultante.
 */
string bytesAHex(const vector<unsigned char>& v) {
  stringstream ss;
  for (size_t i = 0; i < v.size(); i++) {
    ss << hex << setw(2) << setfill('0') << (int)v[i];
  }
  return ss.str();
}

/**
 * @brief Coloca 16 bytes en la matriz de estado 4x4 por columnas.
 *
 * Los bytes se colocan columna a columna (no fila a fila), siguiendo
 * el orden definido por el estándar AES. Los primeros 4 bytes llenan
 * la columna 0, los siguientes 4 la columna 1, etc.
 *
 * @param v Vector de 16 bytes de entrada.
 * @param estado Matriz 4x4 donde se almacena el estado.
 */
void ponerEstado(const vector<unsigned char>& v, unsigned char estado[4][4]) {
  int k = 0;
  for (int c = 0; c < 4; c++) {
    for (int f = 0; f < 4; f++) {
      estado[f][c] = v[k++];
    }
  }
}

/**
 * @brief Extrae los 16 bytes de la matriz de estado 4x4 en un vector.
 *
 * Lee la matriz por columnas (orden inverso a ponerEstado) y devuelve
 * los 16 bytes en un vector lineal.
 *
 * @param estado Matriz 4x4 del estado actual.
 * @return Vector de 16 bytes extraídos del estado.
 */
vector<unsigned char> sacarEstado(unsigned char estado[4][4]) {
  vector<unsigned char> v;
  for (int c = 0; c < 4; c++) {
    for (int f = 0; f < 4; f++) {
      v.push_back(estado[f][c]);
    }
  }
  return v;
}

/**
 * @brief Convierte la matriz de estado a su representación hexadecimal.
 *
 * Combina sacarEstado y bytesAHex para obtener directamente una cadena
 * hexadecimal a partir de la matriz de estado.
 *
 * @param estado Matriz 4x4 del estado actual.
 * @return Cadena hexadecimal de 32 caracteres representando el estado.
 */
string estadoAHex(unsigned char estado[4][4]) {
  return bytesAHex(sacarEstado(estado));
}

/**
 * @brief Multiplica un byte por 2 en el campo finito GF(2^8).
 *
 * Desplaza el byte un bit a la izquierda. Si el bit más significativo
 * estaba activado (el byte era >= 128), aplica XOR con 0x1B (el polinomio
 * irreducible del estándar AES) para mantener el resultado en el rango 0-255.
 *
 * @param x Byte a multiplicar por 2.
 * @return Resultado de la multiplicación en GF(2^8).
 */
unsigned char por2(unsigned char x) {
  if (x & 0x80) return (x << 1) ^ 0x1b;
  return x << 1;
}

/**
 * @brief Multiplica un byte por 3 en el campo finito GF(2^8).
 *
 * Aprovecha que 3 = 2 + 1 en GF(2^8), por lo que multiplicar por 3
 * es equivalente a (multiplicar por 2) XOR (el byte original).
 *
 * @param x Byte a multiplicar por 3.
 * @return Resultado de la multiplicación en GF(2^8).
 */
unsigned char por3(unsigned char x) {
  return por2(x) ^ x;
}

/**
 * @brief Paso AddRoundKey: mezcla el estado con una subclave mediante XOR.
 *
 * Recorre la matriz de estado por columnas y aplica XOR byte a byte
 * con la subclave de la ronda correspondiente. Es la operación que
 * introduce la clave secreta en el proceso de cifrado.
 *
 * @param estado Matriz 4x4 del estado (se modifica in situ).
 * @param clave Vector de 16 bytes con la subclave de la ronda.
 */
void addRoundKey(unsigned char estado[4][4], const vector<unsigned char>& clave) {
  int k = 0;
  for (int c = 0; c < 4; c++) {
    for (int f = 0; f < 4; f++) {
      estado[f][c] ^= clave[k++];
    }
  }
}

/**
 * @brief Paso SubBytes: sustituye cada byte del estado usando la S-Box.
 *
 * Recorre los 16 bytes de la matriz de estado y reemplaza cada uno
 * por su correspondiente en la tabla SBOX. Introduce confusión en el
 * cifrado haciendo que la relación entrada-salida sea no lineal.
 *
 * @param estado Matriz 4x4 del estado (se modifica in situ).
 */
void subBytes(unsigned char estado[4][4]) {
  for (int f = 0; f < 4; f++) {
    for (int c = 0; c < 4; c++) {
      estado[f][c] = SBOX[estado[f][c]];
    }
  }
}

/**
 * @brief Paso ShiftRows: rota las filas del estado hacia la izquierda.
 *
 * Cada fila se desplaza un número diferente de posiciones:
 * - Fila 0: sin desplazamiento.
 * - Fila 1: 1 posición a la izquierda.
 * - Fila 2: 2 posiciones a la izquierda.
 * - Fila 3: 3 posiciones a la izquierda (equivale a 1 a la derecha).
 *
 * Esto dispersa los bytes entre columnas, contribuyendo a la difusión.
 *
 * @param estado Matriz 4x4 del estado (se modifica in situ).
 */
void shiftRows(unsigned char estado[4][4]) {
  unsigned char temp;

  temp = estado[1][0];
  estado[1][0] = estado[1][1];
  estado[1][1] = estado[1][2];
  estado[1][2] = estado[1][3];
  estado[1][3] = temp;

  unsigned char t0 = estado[2][0];
  unsigned char t1 = estado[2][1];
  estado[2][0] = estado[2][2];
  estado[2][1] = estado[2][3];
  estado[2][2] = t0;
  estado[2][3] = t1;

  temp = estado[3][3];
  estado[3][3] = estado[3][2];
  estado[3][2] = estado[3][1];
  estado[3][1] = estado[3][0];
  estado[3][0] = temp;
}

/**
 * @brief Paso MixColumns: mezcla cada columna del estado.
 *
 * Para cada columna, calcula 4 bytes nuevos donde cada uno depende
 * de los 4 bytes originales de esa columna, usando multiplicaciones
 * por 2 y por 3 en GF(2^8) combinadas con XOR. Esto logra que un
 * cambio en un solo byte afecte a toda la columna (difusión).
 *
 * @param estado Matriz 4x4 del estado (se modifica in situ).
 */
void mixColumns(unsigned char estado[4][4]) {
  for (int c = 0; c < 4; c++) {
    unsigned char a0 = estado[0][c];
    unsigned char a1 = estado[1][c];
    unsigned char a2 = estado[2][c];
    unsigned char a3 = estado[3][c];

    estado[0][c] = por2(a0) ^ por3(a1) ^ a2 ^ a3;
    estado[1][c] = a0 ^ por2(a1) ^ por3(a2) ^ a3;
    estado[2][c] = a0 ^ a1 ^ por2(a2) ^ por3(a3);
    estado[3][c] = por3(a0) ^ a1 ^ a2 ^ por2(a3);
  }
}

/**
 * @brief Rota una palabra de 4 bytes una posición a la izquierda.
 *
 * Desplaza los bytes de forma circular: [a, b, c, d] -> [b, c, d, a].
 * Se usa durante la expansión de clave en las posiciones múltiplo de 4.
 *
 * @param w Palabra de 4 bytes a rotar.
 * @return Palabra rotada.
 */
vector<unsigned char> rotWord(vector<unsigned char> w) {
  unsigned char temp = w[0];
  w[0] = w[1];
  w[1] = w[2];
  w[2] = w[3];
  w[3] = temp;
  return w;
}

/**
 * @brief Aplica la S-Box a cada byte de una palabra de 4 bytes.
 *
 * Sustituye cada uno de los 4 bytes usando la tabla SBOX,
 * igual que SubBytes pero solo para una palabra individual.
 * Se usa durante la expansión de clave.
 *
 * @param w Palabra de 4 bytes a sustituir.
 * @return Palabra con los bytes sustituidos.
 */
vector<unsigned char> subWord(vector<unsigned char> w) {
  for (int i = 0; i < 4; i++) {
    w[i] = SBOX[w[i]];
  }
  return w;
}

/**
 * @brief Expande la clave original de 16 bytes en 11 subclaves de 16 bytes.
 *
 * A partir de la clave original genera 44 palabras de 4 bytes (w[0..43]):
 * - w[0..3]: se obtienen directamente de la clave original.
 * - w[4..43]: cada palabra se calcula como XOR entre w[i-4] y w[i-1].
 *   En las posiciones múltiplo de 4, se aplica además RotWord, SubWord
 *   y XOR con la constante RCON correspondiente.
 *
 * Finalmente agrupa las 44 palabras en 11 subclaves de 16 bytes cada una,
 * una para la ronda inicial y una para cada una de las 10 rondas.
 *
 * @param clave Vector de 16 bytes con la clave original.
 * @return Vector de 11 subclaves, cada una de 16 bytes.
 */
vector<vector<unsigned char>> expandirClave(const vector<unsigned char>& clave) {
  vector<vector<unsigned char>> w(44, vector<unsigned char>(4));

  for (int i = 0; i < 4; i++) {
    w[i][0] = clave[4 * i];
    w[i][1] = clave[4 * i + 1];
    w[i][2] = clave[4 * i + 2];
    w[i][3] = clave[4 * i + 3];
  }

  for (int i = 4; i < 44; i++) {
    vector<unsigned char> temp = w[i - 1];

    if (i % 4 == 0) {
      temp = rotWord(temp);
      temp = subWord(temp);
      temp[0] ^= RCON[i / 4 - 1];
    }

    for (int j = 0; j < 4; j++) {
      w[i][j] = w[i - 4][j] ^ temp[j];
    }
  }

  vector<vector<unsigned char>> subclaves(11, vector<unsigned char>(16));

  for (int r = 0; r < 11; r++) {
    int k = 0;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        subclaves[r][k++] = w[r * 4 + i][j];
      }
    }
  }

  return subclaves;
}

/**
 * @brief Función principal que ejecuta el cifrado AES-128.
 *
 * Flujo del programa:
 * 1. Solicita al usuario la clave y el bloque en hexadecimal (32 caracteres cada uno).
 * 2. Valida que ambas entradas sean hexadecimales válidas de 32 caracteres.
 * 3. Convierte las entradas a bytes y expande la clave en 11 subclaves.
 * 4. Ejecuta la ronda 0: AddRoundKey con la subclave inicial.
 * 5. Ejecuta las rondas 1-9: SubBytes + ShiftRows + MixColumns + AddRoundKey.
 * 6. Ejecuta la ronda 10: SubBytes + ShiftRows + AddRoundKey (sin MixColumns).
 * 7. Muestra el texto cifrado resultante.
 *
 * @return 0 si el cifrado se completó correctamente, 1 si la entrada es inválida.
 */
int main() {
  string claveHex;
  string bloqueHex;

  cout << "Clave: ";
  cin >> claveHex;
  cout << "Bloque de Texto Original: ";
  cin >> bloqueHex;

  if (!esHex(claveHex) || !esHex(bloqueHex)) {
    cout << "Error: debes introducir 16 bytes en hexadecimal (32 caracteres)." << endl;
    return 1;
  }

  vector<unsigned char> clave = hexABytes(claveHex);
  vector<unsigned char> bloque = hexABytes(bloqueHex);
  vector<vector<unsigned char>> subclaves = expandirClave(clave);

  unsigned char estado[4][4];
  ponerEstado(bloque, estado);

  addRoundKey(estado, subclaves[0]);
  cout << endl;
  cout << "R0 (Subclave = " << bytesAHex(subclaves[0]) << ") = " << estadoAHex(estado) << endl;

  for (int ronda = 1; ronda <= 9; ronda++) {
    subBytes(estado);
    shiftRows(estado);
    mixColumns(estado);
    addRoundKey(estado, subclaves[ronda]);

    cout << "R" << ronda << " (Subclave = " << bytesAHex(subclaves[ronda]) << ") = "
         << estadoAHex(estado) << endl;
  }

  subBytes(estado);
  shiftRows(estado);
  addRoundKey(estado, subclaves[10]);

  cout << "R10 (Subclave = " << bytesAHex(subclaves[10]) << ") = "
       << estadoAHex(estado) << endl;

  cout << endl;
  cout << "Bloque de Texto Cifrado: " << estadoAHex(estado) << endl;

  return 0;
}
