/**==============================================================================
    Admin8.2.1 - Command.hpp
    Proposito: Estructura de comando y funciones para detectar comandos,
               sujetos y objetos a partir de una oración.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Depende de Word y Sentence, pero no de persistencia.
==============================================================================*/

#ifndef ADMIN821_COMMAND_HPP
#define ADMIN821_COMMAND_HPP

#include "../common/types.hpp"
#include "Word.hpp"
#include "Sentence.hpp"
#include <string>
#include <vector>
#include <optional>

class Command {
public:
    Command() : action_(TipoCommand::LEARN), success_(false) {}
    Command(TipoCommand act, std::string sub, std::string obj)
        : action_(act), subject_(std::move(sub)), object_(std::move(obj)), success_(false) {}

    void setSuccess(bool suc) { success_ = suc; }
    TipoCommand getAction() const { return action_; }
    const std::string& getSubject() const { return subject_; }
    const std::string& getObject() const { return object_; }
    bool isSuccess() const { return success_; }

private:
    TipoCommand action_;
    std::string subject_;
    std::string object_;
    bool success_;
};

// Detectar tipo de comando a partir de una frase
std::optional<TipoCommand> detectCommandFromPhrase(const std::string& phrase);

// Detectar sujetos (sustantivos tras preposición "a" o similar)
std::vector<std::string> detectSubjects(const std::vector<Word>& words);
std::vector<std::string> detectSubjects(const Sentence& sentence);

// Detectar objetos (sustantivos tras el verbo que no sean sujetos)
std::vector<std::string> detectObjects(const std::vector<Word>& words);
std::vector<std::string> detectObjects(const Sentence& sentence);

#endif // ADMIN821_COMMAND_HPP
