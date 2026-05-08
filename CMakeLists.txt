cmake_minimum_required(VERSION 3.14)   # Necesario para FetchContent
project(NLPEngine VERSION 1.0)

# Opción para forzar la descarga de SQLite3 en lugar de usar la del sistema
option(FORCE_DOWNLOAD_SQLITE3 "Forzar descarga y compilación de SQLite3 desde fuente" OFF)

# Estándar C++17 obligatorio
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configuración por defecto: Release si no se especifica
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Tipo de compilación" FORCE)
endif()

# Advertencias útiles (compatibles con GCC/Clang/MSVC)
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)
endif()

# -------------------------------------------------------------------
# 1. Intentar encontrar SQLite3 en el sistema
# -------------------------------------------------------------------
set(SQLITE3_FOUND_SYSTEM FALSE)

if(NOT FORCE_DOWNLOAD_SQLITE3)
    find_package(SQLite3 QUIET)
    if(SQLite3_FOUND)
        message(STATUS "SQLite3 encontrado en el sistema: ${SQLite3_INCLUDE_DIRS}")
        set(SQLITE3_FOUND_SYSTEM TRUE)
    else()
        message(STATUS "SQLite3 no encontrado en el sistema. Se descargará y compilará.")
    endif()
endif()

# -------------------------------------------------------------------
# 2. Si no se encontró o se fuerza la descarga, obtener SQLite3 desde fuente
# -------------------------------------------------------------------
if(NOT SQLITE3_FOUND_SYSTEM OR FORCE_DOWNLOAD_SQLITE3)
    include(FetchContent)

    # Versión actual de SQLite3 (amalgamación) - actualizar si es necesario
    set(SQLITE3_VERSION "3460000")  # 3.46.0
    set(SQLITE3_YEAR "2024")
    set(SQLITE3_URL "https://www.sqlite.org/${SQLITE3_YEAR}/sqlite-amalgamation-${SQLITE3_VERSION}.zip")

    FetchContent_Declare(
        sqlite3
        URL ${SQLITE3_URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    FetchContent_GetProperties(sqlite3)
    if(NOT sqlite3_POPULATED)
        FetchContent_Populate(sqlite3)
    endif()

    # Compilar SQLite3 como biblioteca estática
    add_library(sqlite3 STATIC "${sqlite3_SOURCE_DIR}/sqlite3.c")
    target_include_directories(sqlite3 PUBLIC "${sqlite3_SOURCE_DIR}")
    target_compile_definitions(sqlite3 PRIVATE SQLITE_THREADSAFE=1)

    set(SQLITE3_INCLUDE_DIRS "${sqlite3_SOURCE_DIR}")
    set(SQLITE3_LIBRARIES sqlite3)
    message(STATUS "SQLite3 compilado desde fuente en: ${sqlite3_SOURCE_DIR}")
else()
    set(SQLITE3_LIBRARIES SQLite::SQLite3)
endif()

# -------------------------------------------------------------------
# 3. Fuentes del proyecto
# -------------------------------------------------------------------
set(SOURCES
    src/api/NLPEngine.cpp
    src/core/Command.cpp
    src/core/Dialogue.cpp
    src/core/inferenceEngine.cpp
    src/core/Pattern.cpp
    src/core/Sentence.cpp
    src/core/Word.cpp
    src/db/DatabaseManager.cpp
    src/db/DialogueRepository.cpp
    src/db/PatternRepository.cpp
    src/db/SentenceRepository.cpp
    src/db/TemplateRepository.cpp
    src/db/WordRepository.cpp
    src/dialogue/ChunkCorrelator.cpp
    src/dialogue/ContextualCorrelator.cpp
    src/dialogue/PatternCorrelator.cpp
    src/nlp/Classifier.cpp
    src/nlp/Morphology.cpp
    src/nlp/Refiner.cpp
    src/nlp/TagStats.cpp
    src/nlp/Tokenizer.cpp
    src/utils/Chunker.cpp
    src/utils/LearningHelpers.cpp
    src/utils/PatternUtils.cpp
    src/utils/ResponseTemplates.cpp
    src/utils/SentenceUtils.cpp
    src/utils/SlotFiller.cpp
    src/utils/StringConversions.cpp
    main.cpp
)

# -------------------------------------------------------------------
# 4. Ejecutable y enlaces
# -------------------------------------------------------------------
add_executable(nlp_engine ${SOURCES})

target_include_directories(nlp_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nlp_engine ${SQLITE3_LIBRARIES})

# Opción para enlace estático frente a dinámico (útil en despliegues)
if(UNIX AND NOT APPLE AND NOT FORCE_DOWNLOAD_SQLITE3)
    # En Linux, se puede preferir enlace estático si la biblioteca del sistema lo permite
    # Nota: algunas distribuciones no proporcionan libsqlite3.a, entonces se usa dinámico.
    target_link_options(nlp_engine PRIVATE -static-libstdc++ -static-libgcc)
endif()

# Mensaje final de configuración
message(STATUS "===== Configuración completada =====")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(SQLITE3_FOUND_SYSTEM)
    message(STATUS "SQLite3: usando biblioteca del sistema")
else()
    message(STATUS "SQLite3: usando biblioteca compilada desde fuente")
endif()
