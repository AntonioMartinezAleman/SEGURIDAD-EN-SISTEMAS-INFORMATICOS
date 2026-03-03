/**
 * @file practica.cpp
 * @brief Implementación del Generador C/A usado en el sistema GPS.
 *
 * El código C/A (Coarse/Acquisition) es una secuencia pseudoaleatoria
 * generada mediante dos registros de desplazamiento con retroalimentación
 * lineal (LFSR) denominados G1 y G2, combinados mediante un selector de fase.
 *
 * @details
 * - G1: polinomio de retroalimentación 1 + x^3 + x^10
 * - G2: polinomio de retroalimentación 1 + x^2 + x^3 + x^6 + x^8 + x^9 + x^10
 *
 * Práctica 4 — CryptULL, Universidad de La Laguna
 */

#include <iostream>
#include <vector>
#include <map>

/**
 * @brief Tabla de taps del registro G2 para cada ID de satélite PRN (1-32).
 *
 * Cada entrada mapea un ID de satélite a un par de posiciones (1-indexadas)
 * usadas como selector de fase en la salida de G2.
 */
std::map<int, std::pair<int,int>> g2_taps = {
    {1, {2,6}}, {2, {3,7}}, {3, {4,8}}, {4, {5,9}}, {5, {1,9}},
    {6, {2,10}}, {7, {1,8}}, {8, {2,9}}, {9, {3,10}}, {10, {2,3}},
    {11, {3,4}}, {12, {5,6}}, {13, {6,7}}, {14, {7,8}}, {15, {8,9}},
    {16, {9,10}}, {17, {1,4}}, {18, {2,5}}, {19, {3,6}}, {20, {4,7}},
    {21, {5,8}}, {22, {6,9}}, {23, {1,3}}, {24, {4,6}}, {25, {5,7}},
    {26, {6,8}}, {27, {7,9}}, {28, {8,10}}, {29, {1,6}}, {30, {2,7}},
    {31, {3,8}}, {32, {4,9}}
};

/**
 * @brief Calcula el bit de retroalimentación del registro G1.
 *
 * G1 usa el polinomio 1 + x^3 + x^10, por lo que la retroalimentación
 * es el XOR de las posiciones 3 y 10 (1-indexadas).
 *
 * @param reg Estado actual del registro G1 (10 bits).
 * @return Bit de retroalimentación (0 o 1).
 */
int g1_feedback(std::vector<int>& reg) {
    return reg[3-1] ^ reg[10-1];
}

/**
 * @brief Calcula el bit de retroalimentación del registro G2.
 *
 * G2 usa el polinomio 1 + x^2 + x^3 + x^6 + x^8 + x^9 + x^10,
 * por lo que la retroalimentación es el XOR de las posiciones 2,3,6,8,9,10.
 *
 * @param reg Estado actual del registro G2 (10 bits).
 * @return Bit de retroalimentación (0 o 1).
 */
int g2_feedback(std::vector<int>& reg) {
    return reg[2-1] ^ reg[3-1] ^ reg[6-1] ^ reg[8-1] ^ reg[9-1] ^ reg[10-1];
}

/**
 * @brief Imprime el estado actual de ambos registros y el bit C/A generado.
 *
 * @param g1  Estado del registro G1.
 * @param fb1 Bit de retroalimentación calculado para G1.
 * @param g2  Estado del registro G2.
 * @param fb2 Bit de retroalimentación calculado para G2.
 * @param ca  Bit C/A producido en este ciclo.
 */
void print_state(std::vector<int>& g1, int fb1, std::vector<int>& g2, int fb2, int ca) {
    for (int b : g1) std::cout << b << " ";
    std::cout << "| " << fb1 << " |";
    for (int b : g2) std::cout << " " << b;
    std::cout << " | " << fb2 << " | " << ca << std::endl;
}

/**
 * @brief Genera la secuencia C/A para un satélite GPS dado.
 *
 * Combina las salidas de los registros G1 y G2 mediante el selector de fase
 * definido por los taps asociados al ID de satélite para producir la secuencia
 * pseudoaleatoria C/A.
 *
 * @param prn_id  Identificador del satélite (1-32).
 * @param length  Número de bits a generar.
 * @param verbose Si es true, imprime el estado de los registros en cada ciclo.
 * @return Vector con la secuencia C/A generada, o vacío si el PRN ID es inválido.
 */
std::vector<int> generate_ca(int prn_id, int length, bool verbose = false) {
    if (g2_taps.find(prn_id) == g2_taps.end()) {
        std::cerr << "Invalid PRN ID: " << prn_id << std::endl;
        return {};
    }

    auto [tap1, tap2] = g2_taps[prn_id];

    // Inicializar ambos registros a todo unos
    std::vector<int> g1(10, 1);
    std::vector<int> g2_reg(10, 1);

    std::vector<int> ca_sequence;

    if (verbose) {
        std::cout << "LFSR1 (A-J)                    | FB1 | LFSR2 (N-W)                    | FB2 | C/A" << std::endl;
        std::cout << std::string(90, '-') << std::endl;
    }

    for (int i = 0; i < length; i++) {
        // Salida de G2 usando los taps del selector de fase
        int g2_out = g2_reg[tap1-1] ^ g2_reg[tap2-1];

        // Bit C/A = G1[10] XOR selector de fase G2
        int ca_bit = g1[10-1] ^ g2_out;
        ca_sequence.push_back(ca_bit);

        // Calcular retroalimentaciones antes de desplazar
        int fb1 = g1_feedback(g1);
        int fb2 = g2_feedback(g2_reg);

        if (verbose) {
            print_state(g1, fb1, g2_reg, fb2, ca_bit);
        }

        // Desplazar G1 (shift right, insertar retroalimentación en posición 0)
        for (int j = 9; j > 0; j--) g1[j] = g1[j-1];
        g1[0] = fb1;

        // Desplazar G2
        for (int j = 9; j > 0; j--) g2_reg[j] = g2_reg[j-1];
        g2_reg[0] = fb2;
    }

    return ca_sequence;
}

/**
 * @brief Punto de entrada del programa.
 *
 * Solicita al usuario el ID del satélite y la longitud de la secuencia,
 * genera la secuencia C/A correspondiente y la muestra por pantalla.
 *
 * @return 0 si la ejecución fue correcta, 1 en caso de error de entrada.
 */
int main() {
    int prn_id, length;

    std::cout << "Generador C/A de GPS" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "ID del satelite (1-32): ";
    std::cin >> prn_id;
    std::cout << "Longitud de la secuencia de salida: ";
    std::cin >> length;

    if (prn_id < 1 || prn_id > 32) {
        std::cerr << "Error: ID de satelite debe estar entre 1 y 32." << std::endl;
        return 1;
    }
    if (length <= 0) {
        std::cerr << "Error: La longitud debe ser un numero positivo." << std::endl;
        return 1;
    }

    std::cout << std::endl;
    std::vector<int> seq = generate_ca(prn_id, length, true);

    std::cout << std::endl << "Secuencia C/A PRN" << prn_id << ": ";
    for (int b : seq) std::cout << b;
    std::cout << std::endl;

    return 0;
}