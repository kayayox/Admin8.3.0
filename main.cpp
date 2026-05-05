/**==============================================================================
    Admin8.2.1 - main.cpp (ejemplo de uso de la API)
    Propósito: Demostración interactiva de todas las capacidades del motor NLP.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "src/api/NLPEngine.hpp"
#include "src/nlp/Tokenizer.hpp"          // splitIntoSentences
#include "src/core/Command.hpp"            // detectCommandFromPhrase
#include "src/utils/StringConversions.hpp" // tipoToString (opcional)

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cctype>

// ============================================================================
// Funciones auxiliares
// ============================================================================

void printWordInfo(const WordInfo& info) {
    std::cout << "Palabra: " << info.word << "\n"
              << "  Tipo: " << info.tipo << " (confianza: " << info.confianza << ")\n"
              << "  Significado: " << info.significado << "\n"
              << "  Cantidad: " << info.cantidad << ", Tiempo: " << info.tiempo
              << ", Género: " << info.genero << ", Persona: " << info.persona
              << ", Grado: " << info.grado << "\n";
    for (const auto& rel : info.relacionadas) {
        std::cout << "  " << rel.first << " = " << rel.second << std::endl;
    }
}

void printPredictions(const std::vector<Prediction>& preds) {
    if (preds.empty()) {
        std::cout << "  No hay predicciones disponibles.\n";
        return;
    }
    std::cout << "  Predicciones:\n";
    for (size_t i = 0; i < preds.size(); ++i) {
        std::cout << "    " << i + 1 << ". '" << preds[i].word
                  << "' (probabilidad: " << preds[i].probability << ")\n";
    }
}

bool askYesNo(const std::string& question) {
    std::string answer;
    while (true) {
        std::cout << question << " (s/n): ";
        std::getline(std::cin, answer);
        if (answer.empty()) continue;
        char first = std::tolower(answer[0]);
        if (first == 's') return true;
        if (first == 'n') return false;
        std::cout << "Respuesta no válida. Por favor responda 's' o 'n'.\n";
    }
}

bool learnFromFile(NLPEngine& engine, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Advertencia: No se pudo abrir el archivo '" << filename << "'.\n";
        return false;
    }
    std::string contenido;
    file.seekg(0, std::ios::end);
    contenido.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&contenido[0], contenido.size());
    file.close();

    auto oraciones = splitIntoSentences(contenido);
    int sentenceCount = 0;
    for (auto& oracion : oraciones) {
        if (oracion.empty()) continue;
        engine.learnText(oracion);
        engine.processSentence(oracion);
        sentenceCount++;
        std::cout << "Aprendida oración " << sentenceCount << ": \"" << oracion << "\"\n";
    }
    std::cout << "Total de " << sentenceCount << " oraciones aprendidas desde '" << filename << "'.\n";
    return true;
}

// ============================================================================
// Menú principal
// ============================================================================

void mostrarMenu() {
    std::cout << "\n=== MENÚ PRINCIPAL ===\n"
              << "1. Aprender de archivo\n"
              << "2. Aprender una frase\n"
              << "3. Procesar una oración (clasificar + aprender)\n"
              << "4. Mostrar información de una palabra\n"
              << "5. Corregir manualmente una palabra\n"
              << "6. Reprocesar última oración (tras corrección)\n"
              << "7. Predicción interactiva (bucle)\n"
              << "8. Generar respuesta a una premisa\n"
              << "0. Salir\n"
              << "Opción: ";
}

int main() {
    // Inicializar motor con tres bases de datos
    NLPEngine engine;
    std::string semanticDb = "nlp_semantic.db";
    std::string patternDb = "nlp_patterns.db";
    std::string temporalDb = ":memory:";

    if (!engine.initialize(semanticDb, patternDb, temporalDb)) {
        std::cerr << "Error fatal: No se pudo inicializar el motor NLP.\n";
        return 1;
    }
    std::cout << "Motor NLP inicializado correctamente.\n";

    int opcion;
    do {
        mostrarMenu();
        std::cin >> opcion;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (opcion) {
            case 1: {
                std::cout << "Nombre del archivo (ej: corpus.txt): ";
                std::string archivo;
                std::getline(std::cin, archivo);
                if (!archivo.empty()) learnFromFile(engine, archivo);
                else std::cout << "No se especificó archivo.\n";
                break;
            }
            case 2: {
                std::cout << "Frase para aprender: ";
                std::string frase;
                std::getline(std::cin, frase);
                if (!frase.empty()) {
                    engine.learnText(frase);
                    engine.processSentence(frase);
                    auto cmd = detectCommandFromPhrase(frase);
                    if (cmd.has_value()) {
                        std::cout << "Comando detectado: " << static_cast<int>(cmd.value()) << std::endl;
                    } else {
                        std::cout << "Sin comando detectable.\n";
                    }
                    std::cout << "Frase aprendida.\n";
                }
                break;
            }
            case 3: {
                std::cout << "Oración a procesar: ";
                std::string oracion;
                std::getline(std::cin, oracion);
                if (!oracion.empty()) {
                    auto infos = engine.processSentence(oracion);
                    std::cout << "Clasificación:\n";
                    for (const auto& info : infos) printWordInfo(info);
                }
                break;
            }
            case 4: {
                std::cout << "Palabra: ";
                std::string palabra;
                std::getline(std::cin, palabra);
                if (!palabra.empty()) {
                    WordInfo info = engine.getWordInfo(palabra);
                    printWordInfo(info);
                }
                break;
            }
            case 5: {
                std::cout << "Palabra a corregir: ";
                std::string palabra;
                std::getline(std::cin, palabra);
                std::cout << "Tipo correcto (Sustantivo, Verbo, Adjetivo, Adverbio, Preposición, Conjunción, Artículo, Pronombre): ";
                std::string tipo;
                std::getline(std::cin, tipo);
                if (!palabra.empty() && !tipo.empty()) {
                    engine.correctWord(palabra, tipo);
                    std::cout << "Corrección guardada. Use opción 6 para reprocesar la última oración y actualizar estadísticas.\n";
                }
                break;
            }
            case 6: {
                engine.reprocessLastSentence();
                std::cout << "Última oración reprocesada (estadísticas actualizadas).\n";
                break;
            }
            case 7: {
                std::cout << "--- MODO PREDICCIÓN ---\n";
                std::cout << "Frase inicial (mínimo 2 palabras): ";
                std::string input;
                std::getline(std::cin, input);
                std::string currentPhrase = input;
                const int MAX_ITER = 15;

                for (int iter = 1; iter <= MAX_ITER; ++iter) {
                    std::cout << "\n=== Iteración " << iter << " ===\n";
                    auto preds = engine.predictNext(currentPhrase);
                    printPredictions(preds);

                    if (preds.empty()) {
                        std::cout << "No hay predicciones. Fin del bucle.\n";
                        break;
                    }

                    std::string predicted = preds[0].word;
                    double bestProb = preds[0].probability;

                    if (bestProb < 0.5 && preds.size() > 1) {
                        std::cout << "Confianza baja. Se mostrarán opciones.\n";
                        for (size_t i = 0; i < preds.size() && i < 5; ++i)
                            std::cout << "   " << i + 1 << ": " << preds[i].word << " (" << preds[i].probability << ")\n";
                        std::cout << "Elija número (1-" << std::min(5, (int)preds.size()) << ") o 0 para ingresar manual: ";
                        int elec;
                        std::cin >> elec;
                        std::cin.ignore();
                        if (elec >= 1 && elec <= (int)preds.size())
                            predicted = preds[elec - 1].word;
                        else if (elec == 0) {
                            std::cout << "Palabra correcta: ";
                            std::getline(std::cin, predicted);
                        }
                    } else {
                        if (!askYesNo("¿Es correcta la predicción '" + predicted + "'?")) {
                            std::cout << "Ingrese la palabra correcta: ";
                            std::getline(std::cin, predicted);
                        }
                    }

                    // Aprender la secuencia correcta
                    std::string correctedSentence = currentPhrase + " " + predicted;
                    engine.processSentence(correctedSentence);
                    std::cout << "Aprendida: " << correctedSentence << "\n";
                    currentPhrase += " " + predicted;

                    // Opcional: corrección de clasificación
                    if (askYesNo("¿Corregir la clasificación de esta palabra?")) {
                        std::cout << "Nuevo tipo: ";
                        std::string newType;
                        std::getline(std::cin, newType);
                        engine.correctWord(predicted, newType);
                        engine.reprocessLastSentence();

                        WordInfo info = engine.getWordInfo(predicted);
                        std::cout << "Después de corrección:\n";
                        printWordInfo(info);
                    }
                }
                std::cout << "\nFrase final generada: " << currentPhrase << "\n";
                break;
            }
            case 8: {
                std::cout << "Premisa (oración de entrada): ";
                std::string premisa;
                std::getline(std::cin, premisa);
                if (!premisa.empty()) {
                    std::string respuesta = engine.generateResponse(premisa);
                    std::cout << "Hipótesis generada: " << respuesta << "\n";
                }
                break;
            }
            case 0:
                std::cout << "Saliendo...\n";
                break;
            default:
                std::cout << "Opción no válida.\n";
        }
    } while (opcion != 0);

    engine.shutdown();
    return 0;
}
