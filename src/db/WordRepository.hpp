/**==============================================================================
    Admin8.2.1 - WordRepository.hpp
    Proposito: Repositorio para persistir entidades Word en la base de datos semántica.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Depende de DatabaseManager y de la entidad Word (sin métodos save/load).
==============================================================================*/

#ifndef ADMIN821_WORD_REPOSITORY_HPP
#define ADMIN821_WORD_REPOSITORY_HPP

#include "../core/Word.hpp"
#include <string>

class WordRepository {
public:
    // Ruta de la base de datos semántica.
    // Se debe llamar a setDatabasePath antes de las operaciones (o se puede pasar por parámetro).
    static void setDatabasePath(const std::string& path);
    static std::string getDatabasePath();

    static void initializeTables();
    // Guardar palabra (insert o update)
    static void save(const Word& word);

    // Cargar palabra por su texto. Retorna true si se encontró y se llena 'outWord'.
    static bool load(const std::string& palabra, Word& outWord);

    // Verificar si existe
    static bool exists(const std::string& palabra);

    // Eliminar palabra (opcional)
    static bool remove(const std::string& palabra);

private:
    static std::string dbPath_;
    static int getOrCreateWordId(const Word& word);  // auxiliar
};

#endif // ADMIN821_WORD_REPOSITORY_HPP
