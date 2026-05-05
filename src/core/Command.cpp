/**==============================================================================
    Admin8.2.2 - Command.cpp
    Proposito: Implementación de detección de comandos, sujetos y objetos.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Basado en listas estáticas de verbos de comando y reglas sintácticas.
==============================================================================*/

#include "Command.hpp"
#include "../db/WordRepository.hpp"
#include <unordered_map>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace {
    // Mapa de verbos de comando
    const std::unordered_map<TipoCommand, std::vector<std::string>> COMMAND_VERBS = {
        { TipoCommand::DO,       {"hacer", "haz", "realiza", "ejecuta", "efectúa", "efectua"} },
        { TipoCommand::ANSWER,   {"responde", "contesta", "di", "contestame"} },
        { TipoCommand::ASK,      {"pregunta", "consulta", "interroga", "averigua"} },
        { TipoCommand::TASK,     {"asigna", "encarga", "encomienda", "manda", "delega"} },
        { TipoCommand::BID,      {"puja", "ofrece", "licita", "apuesta"} },
        { TipoCommand::GO,       {"ve", "anda", "navega", "dirígete", "dirigete", "accede", "desplázate", "desplazate"} },
        { TipoCommand::REPORT,   {"informa", "reporta", "notifica"} },
        { TipoCommand::MOVE,     {"mueve", "desplaza", "traslada"} },
        { TipoCommand::REPEAT,   {"repite", "vuélvelo", "vuelvelo", "repetir", "repite"} },
        { TipoCommand::FIND,     {"busca", "encuentra", "localiza", "halla"} },
        { TipoCommand::CREATE,   {"crea", "genera", "construye", "redacta", "elabora", "produce", "añade"} },
        { TipoCommand::DELETE,   {"elimina", "borra", "suprime", "quita", "deshaz", "tira", "destruye"} },
        { TipoCommand::UPDATE,   {"actualiza", "modifica", "edita", "cambia", "corrige"} },
        { TipoCommand::SEND,     {"envía", "envia", "manda", "transmite", "notifica", "empuja", "mensajea"} },
        { TipoCommand::CALL,     {"llama", "telefonea", "contacta", "videollama", "comunícate"} },
        { TipoCommand::PLAY,     {"reproduce", "toca", "play"} },
        { TipoCommand::PAUSE,    {"pausa", "suspende", "interrumpe"} },
        { TipoCommand::STOP,     {"para", "detén", "deten", "cancela", "termina", "aborta", "finaliza", "alto"} },
        { TipoCommand::START,    {"inicia", "comienza", "arranca", "lanza", "empieza", "ejecuta"} },
        { TipoCommand::SCHEDULE, {"programa", "agenda", "reserva", "planifica"} },
        { TipoCommand::SET,      {"configura", "ajusta", "establece", "activa", "desactiva", "regula"} },
        { TipoCommand::SHOW,     {"muestra", "enseña", "visualiza", "lista", "despliega"} },
        { TipoCommand::HELP,     {"ayuda", "asiste", "auxilio", "help"} },
        { TipoCommand::OPEN,     {"abre", "desbloquea"} },
        { TipoCommand::CLOSE,    {"cierra", "clausura"} }
    };

    std::unordered_map<std::string, TipoCommand> buildReverseVerbMap() {
        std::unordered_map<std::string, TipoCommand> rev;
        for (const auto& [cmd, verbs] : COMMAND_VERBS) {
            for (const auto& v : verbs) {
                std::string key = v;
                std::transform(key.begin(), key.end(), key.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                rev[key] = cmd;
            }
        }
        return rev;
    }

    const std::unordered_map<std::string, TipoCommand> VERB_TO_COMMAND = buildReverseVerbMap();

    // Auxiliares para detección de sujetos/objetos
    std::vector<std::string> collectNounsAfter(const std::vector<Word>& words, bool stopAtVerb) {
        std::vector<std::string> result;
        bool trigger = false;
        for (const auto& w : words) {
            if (!trigger && w.getTipo() == TipoPalabra::PREP) {
                trigger = true;
                continue;
            }
            if (trigger) {
                if (w.getTipo() == TipoPalabra::SUST || w.getTipo() == TipoPalabra::PRON) {
                    result.push_back(w.getPalabra());
                } else if (stopAtVerb && w.getTipo() == TipoPalabra::VERB) {
                    break;
                }
            }
        }
        return result;
    }

    std::vector<std::string> collectNounsAfterVerb(const std::vector<Word>& words) {
        std::vector<std::string> result;
        bool verbSeen = false;
        bool artSeen = false;
        for (const auto& w : words) {
            if (!verbSeen && w.getTipo() == TipoPalabra::VERB) {
                verbSeen = true;
                continue;
            }
            if (w.getTipo() == TipoPalabra::ART) artSeen = true;
            if ((verbSeen || artSeen) && w.getTipo() == TipoPalabra::SUST) {
                result.push_back(w.getPalabra());
            }
        }
        return result;
    }
} // namespace

std::optional<TipoCommand> detectCommandFromPhrase(const std::string& phrase) {
    if (phrase.empty()) return std::nullopt;
    std::istringstream iss(phrase);
    std::string token;
    while (iss >> token) {
        token.erase(std::remove_if(token.begin(), token.end(),
                                   [](unsigned char c) { return std::ispunct(c); }),
                    token.end());
        if (token.empty()) continue;
        std::transform(token.begin(), token.end(), token.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        auto it = VERB_TO_COMMAND.find(token);
        if (it != VERB_TO_COMMAND.end()) {
            Word w;
            if(WordRepository::load(token,w)){
                if(w.getTipo()==TipoPalabra::VERB)return it->second;
            }
        }
    }
    return std::nullopt;
}

std::vector<std::string> detectSubjects(const std::vector<Word>& words) {
    auto raw = collectNounsAfter(words, true);
    std::vector<std::string> result;
    for (const auto& s : raw) result.push_back(s);
    return result;
}

std::vector<std::string> detectObjects(const std::vector<Word>& words) {
    auto raw = collectNounsAfterVerb(words);
    std::vector<std::string> result;
    for (const auto& o : raw) result.push_back(o);
    auto subjects = detectSubjects(words);
    result.erase(std::remove_if(result.begin(), result.end(),
        [&](const std::string& obj) {
            return std::find(subjects.begin(), subjects.end(), obj) != subjects.end();
        }), result.end());
    return result;
}

// Versiones para Sentence (iteran sobre bloques)
std::vector<std::string> detectSubjects(const Sentence& sentence) {
    std::vector<std::string> result;
    bool trigger = false;
    Block key=sentence.getKey();
    if(key.type!=TipoPalabra::VERB)result.push_back(key.text);
    for (const auto& b : sentence.getBlocks()) {
        if (!trigger && b.type == TipoPalabra::PREP) {
            trigger = true;
            continue;
        }
        if (trigger) {
            if (b.type == TipoPalabra::SUST && b.text!=key.text) {
                result.push_back(b.text);
            } else if (b.type == TipoPalabra::VERB) {
                break;
            }
        }
    }
    return result;
}

std::vector<std::string> detectObjects(const Sentence& sentence) {
    std::vector<std::string> result;
    bool verbSeen = false;
    bool artSeen = false;
    for (const auto& b : sentence.getBlocks()) {
        if (!verbSeen && b.type == TipoPalabra::VERB) {
            verbSeen = true;
            continue;
        }
        if(b.type == TipoPalabra::ART)artSeen = true;
        if ((verbSeen || artSeen) && b.type == TipoPalabra::SUST) {
            result.push_back(b.text);
        }
    }
    auto subjects = detectSubjects(sentence);
    result.erase(std::remove_if(result.begin(), result.end(),
        [&](const std::string& obj) {
            return std::find(subjects.begin(), subjects.end(), obj) != subjects.end();
        }), result.end());
    return result;
}
