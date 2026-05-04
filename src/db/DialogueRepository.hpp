/**==============================================================================
    Admin8.2.1 - DialogueRepository.hpp
    Proposito: Repositorio para persistir diálogos y feedback en la BD semántica.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#ifndef ADMIN821_DIALOGUE_REPOSITORY_HPP
#define ADMIN821_DALOGUE_REPOSITORY_HPP

#include "../core/Dialogue.hpp"
#include <string>

class DialogueRepository {
public:
    static void setDatabasePath(const std::string& path);
    static std::string getDatabasePath();
    static void initializeTables();

    // Guardar un diálogo (premisa e hipótesis deben estar ya guardadas o se guardan automáticamente)
    static void saveDialogue(const Sentence& premise, const Sentence& hypothesis,
                             const Pattern& pattern, float creativity);

    // Registrar feedback de clasificación de palabra
    static void registerFeedback(const std::string& word,
                                 TipoPalabra proposedType,
                                 TipoPalabra correctType,
                                 bool correct);

    // Construir corpus completo a partir de todos los bloques de oraciones almacenadas
    static std::vector<std::string> buildCorpus();

    // Cargar todo el historial de diálogos
    static DialogueHistory loadHistory();

private:
    static std::string dbPath_;
    static int getOrCreateSentenceId(const Sentence& sentence);
};

#endif // ADMIN821_DIALOGUE_REPOSITORY_HPP
