#include "SentenceUtils.hpp"
#include "../db/WordRepository.hpp"
#include "../nlp/Tokenizer.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <regex>

namespace utils {

// Inicialización única del generador aleatorio (para funciones de creatividad)
static const bool s_randInit = []() -> bool {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    return true;
}();

// ----------------------------------------------------------------------------
// parsePremise (extracción mejorada de sujeto, verbo, objeto)
// ----------------------------------------------------------------------------
ParsedPremise parsePremise(const Sentence& premise) {
    ParsedPremise result;
    const auto& blocks = premise.getBlocks();
    bool afterPrep = false;

    // 1. Sujeto tras preposición "a" (simplificado, no se compara texto exacto)
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].type == TipoPalabra::PREP) {
            afterPrep = true;
            continue;
        }
        if (afterPrep && (blocks[i].type == TipoPalabra::SUST || blocks[i].type == TipoPalabra::PRON)) {
            result.subject = blocks[i].text;
            afterPrep = false;
            break;
        }
    }
    // 2. Sujeto general (primer sustantivo si no se encontró antes)
    if (result.subject.empty()) {
        for (const auto& b : blocks) {
            if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::PRON) {
                result.subject = b.text;
                break;
            }
        }
    }

    // 3. Verbo (el primero)
    for (const auto& b : blocks) {
        if (b.type == TipoPalabra::VERB) {
            result.verb = b.text;
            break;
        }
    }

    // 4. Objeto (primer sustantivo después del verbo)
    bool verbPassed = false;
    for (const auto& b : blocks) {
        if (!verbPassed && b.type == TipoPalabra::VERB) {
            verbPassed = true;
            continue;
        }
        if (verbPassed && (b.type == TipoPalabra::SUST || b.type == TipoPalabra::PRON)) {
            result.object = b.text;
            break;
        }
    }

    // 5. Palabras clave (sustantivos + adjetivos)
    for (const auto& b : blocks) {
        if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::PRON || b.type == TipoPalabra::ADJT) {
            result.keywords.push_back(b.text);
        }
    }

    result.patternType = classifySentencePattern(premise.getTypeSequence());
    return result;
}

// ----------------------------------------------------------------------------
// buildSentenceFromText
// ----------------------------------------------------------------------------
Sentence buildSentenceFromText(const std::string& text) {
    auto tokens = tokenize(text);
    std::vector<Word> words;
    for (const auto& tok : tokens) {
        Word w(tok.text);
        WordRepository::load(tok.text, w);  // carga tipo, etc. si existe
        words.push_back(w);
    }
    return Sentence(words);
}

// ----------------------------------------------------------------------------
// applyCreativity (versión básica)
// ----------------------------------------------------------------------------
void applyCreativity(std::string& text, float creativity) {
    if (creativity < 0.3f) return;

    // Toque de duda para creatividad muy alta
    if (creativity > 0.8f &&
        text.find("no") == std::string::npos &&
        text.find('?') == std::string::npos) {
        text = "Quizás " + text;
    }

    // Sustitución aleatoria por sinónimos/relacionados
    if (creativity > 0.5f) {
        std::vector<std::string> words;
        std::istringstream iss(text);
        std::string w;
        while (iss >> w) words.push_back(w);

        for (auto& w : words) {
            Word wordObj;
            if (WordRepository::load(w, wordObj)) {
                const auto& rels = wordObj.getRelated();
                if (!rels.empty() && (rand() % 100) < static_cast<int>(creativity * 100)) {
                    w = rels[0].first;
                    break;  // solo un reemplazo por hipótesis
                }
            }
        }

        std::string rebuilt;
        for (const auto& word : words) {
            if (!rebuilt.empty()) rebuilt += " ";
            rebuilt += word;
        }
        text = rebuilt;
    }
}

// ----------------------------------------------------------------------------
// advancedCreativeTransform (versión más elaborada)
// ----------------------------------------------------------------------------
void advancedCreativeTransform(std::string& text, float creativity,
                               const ParsedPremise& premiseInfo) {
    if (creativity < 0.2f) return;

    // 1. Añadir muletilla al inicio si no existe y creatividad alta
    if (creativity > 0.7f && text.find("quizás") == std::string::npos &&
        text.find("tal vez") == std::string::npos && text.find("¿") != 0) {
        if (rand() % 100 < 40) {
            text = "Quizás " + text;
        }
    }

    // 2. Reemplazar palabras por sinónimos (usando WordRepository)
    if (creativity > 0.4f) {
        std::vector<std::string> words;
        std::istringstream iss(text);
        std::string w;
        while (iss >> w) words.push_back(w);

        for (auto& w : words) {
            Word wordObj;
            if (WordRepository::load(w, wordObj)) {
                const auto& rels = wordObj.getRelated();
                if (!rels.empty() && (rand() % 100) < static_cast<int>(creativity * 40)) {
                    int idx = rand() % rels.size();
                    w = rels[idx].first;
                    break;
                }
            }
        }

        std::string rebuilt;
        for (const auto& word : words) {
            if (!rebuilt.empty()) rebuilt += " ";
            rebuilt += word;
        }
        text = rebuilt;
    }

    // 3. Cambiar signos de puntuación: si es afirmación y creatividad alta, convertir a pregunta
    if (creativity > 0.8f && text.find('?') == std::string::npos &&
        text.find("no") == std::string::npos && text.back() != '?') {
        text = "¿" + text + "?";
    } else if (text.back() != '.' && text.back() != '?' && text.back() != '!') {
        text += ".";
    }

    // 4. Capitalizar primera letra
    if (!text.empty()) {
        text[0] = std::toupper(text[0]);
    }
}

// ----------------------------------------------------------------------------
// computeCreativity
// ----------------------------------------------------------------------------
float computeCreativity(const Sentence& premise, const Sentence& hypothesis,
                        const Pattern& /*pattern*/) {
    std::vector<std::string> premiseWords, hypoWords;
    for (const auto& b : premise.getBlocks()) {
        if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::VERB)
            premiseWords.push_back(b.text);
    }
    for (const auto& b : hypothesis.getBlocks()) {
        if (b.type == TipoPalabra::SUST || b.type == TipoPalabra::VERB)
            hypoWords.push_back(b.text);
    }

    int common = 0;
    for (const auto& pw : premiseWords) {
        if (std::find(hypoWords.begin(), hypoWords.end(), pw) != hypoWords.end())
            common++;
    }

    if (premiseWords.empty() && hypoWords.empty()) return 0.5f;
    float base = static_cast<float>(common) /
                 std::max(premiseWords.size(), hypoWords.size());

    int diff = static_cast<int>(premise.getBlocks().size()) -
               static_cast<int>(hypothesis.getBlocks().size());
    int maxSize = static_cast<int>(std::max(premise.getBlocks().size(),
                                            hypothesis.getBlocks().size()));
    float lenRatio = (maxSize == 0) ? 1.0f
                     : 1.0f - static_cast<float>(std::abs(diff)) / static_cast<float>(maxSize);

    return std::min(0.95f, base * 0.7f + lenRatio * 0.3f);
}

// ----------------------------------------------------------------------------
// applyCreativityToText (wrapper que devuelve una copia)
// ----------------------------------------------------------------------------
std::string applyCreativityToText(const std::string& text, float creativity) {
    std::string result = text;
    // Usamos una ParsedPremise vacía para la transformación avanzada
    ParsedPremise emptyPremise;
    advancedCreativeTransform(result, creativity, emptyPremise);
    return result;
}

} // namespace utils
