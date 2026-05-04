/**==============================================================================
    Admin8.2.1 - SentenceRepository.hpp
    Proposito: Repositorio para persistir entidades Sentence en la base de datos semántica.
    Autor: Soubhi Khayat Najjar
    Año: 2026
    Notas: Depende de DatabaseManager y de la entidad Sentence.
           Las tablas se crean automáticamente al iniciar si no existen.
==============================================================================*/

#ifndef ADMIN821_SENTENCE_REPOSITORY_HPP
#define ADMIN821_SENTENCE_REPOSITORY_HPP

#include "../core/Sentence.hpp"
#include <string>

class SentenceRepository {
public:
    // Ruta de la base de datos semántica
    static void setDatabasePath(const std::string& path);
    static std::string getDatabasePath();

    // Crear las tablas necesarias si no existen (oraciones, bloques)
    static void initializeTables();

    // Guardar una oración: inserta si id == -1, actualiza si id > 0
    // El id se asigna automáticamente en caso de inserción.
    static void save(Sentence& sentence);

    // Cargar por ID
    static Sentence loadById(int id);

    // Cargar por clave (palabra clave y tipo)
    static Sentence loadByKey(const std::string& keyWord, TipoPalabra keyType);

    // Eliminar una oración
    static bool remove(int id);

    // Obtener todas las oraciones (opcional)
    static std::vector<Sentence> loadAll();

private:
    static std::string dbPath_;
    static int getOrCreateOracionId(const Sentence& sentence); // auxiliar
};

#endif // ADMIN821_SENTENCE_REPOSITORY_HPP
