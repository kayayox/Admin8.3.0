/**==============================================================================
    Admin8.2.1 - Morphology.cpp
    Proposito: Implementación de análisis morfológico.
    Autor: Soubhi Khayat Najjar
    Año: 2026
==============================================================================*/

#include "Morphology.hpp"
#include <vector>
#include <algorithm>
#include <cctype>
#include <regex>

namespace morphology {

    // ==================== LISTAS COMUNES ====================
    static const std::vector<std::string> comunes_sustantivos = {
        "casa", "perro", "gato", "hombre", "mujer", "niño", "niña", "padre", "madre", "hermano",
        "hermana", "amigo", "amiga", "trabajo", "escuela", "ciudad", "país", "mundo", "vida", "día",
        "noche", "tiempo", "año", "mes", "mar", "semana", "hora", "minuto", "persona", "familia", "empresa",
        "agua", "aire", "fuego", "tierra", "sol", "luna", "estrella", "cielo", "árbol", "flor",
        "pájaro", "pez", "caballo", "vaca", "cerdo", "oveja", "pato", "gallina", "conejo", "ratón",
        "elefante", "tigre", "león", "oso", "lobo", "zorro", "serpiente", "insecto", "mosca", "abeja",
        "cabeza", "cara", "ojo", "oreja", "nariz", "boca", "diente", "lengua", "cuello", "espalda",
        "brazo", "mano", "dedo", "uña", "pierna", "pie", "rodilla", "corazón", "sangre", "hueso",
        "comida", "bebida", "pan", "queso", "leche", "huevo", "carne", "arroz", "pasta", "fruta",
        "manzana", "pera", "plátano", "naranja", "uva", "fresa", "cereza", "tomate", "patata", "cebolla",
        "mesa", "silla", "cama", "puerta", "ventana", "techo", "pared", "cocina", "baño", "habitación",
        "reloj", "teléfono", "televisor", "ordenador", "coche", "bicicleta", "tren", "avión", "barco", "autobús",
        "calle", "carretera", "camino", "puente", "río", "montaña", "bosque", "playa", "isla", "continente",
        "dinero", "precio", "compras", "tienda", "mercado", "banco", "hospital", "biblioteca", "iglesia", "museo",
        "arte", "música", "canción", "libro", "película", "historia", "idioma", "palabra", "frase", "página",
        "amor", "alegría", "miedo", "dolor", "paz", "guerra", "problema", "solución", "sueño", "recuerdo"
    };
    static const std::vector<std::string> comunes_adjetivos = {
        "bueno", "buena", "malo", "mala", "grande", "pequeño", "pequeña", "nuevo", "nueva", "viejo",
        "vieja", "joven", "alto", "alta", "bajo", "baja", "feliz", "triste", "inteligente", "amable",
        "rojo", "roja", "azul", "verde", "amarillo", "amarilla", "blanco", "blanca", "negro", "negra",
        "gris", "marrón", "naranja", "rosa", "violeta", "oscuro", "oscura", "claro", "clara",
        "largo", "larga", "corto", "corta", "estrecho", "estrecha", "ancho", "ancha", "gordo", "gorda",
        "delgado", "delgada", "redondo", "redonda", "cuadrado", "cuadrada", "recto", "recta", "curvo", "curva",
        "bonito", "bonita", "feo", "fea", "guapo", "guapa", "hermoso", "hermosa", "lindo", "linda",
        "rico", "rica", "pobre", "dulce", "salado", "salada", "amargo", "amarga", "ácido", "ácida",
        "caliente", "frío", "fría", "tibio", "tibia", "seco", "seca", "mojado", "mojada", "húmedo", "húmeda",
        "rápido", "rápida", "lento", "lenta", "fuerte", "débil", "duro", "dura", "blando", "blanda",
        "fácil", "difícil", "sencillo", "sencilla", "complicado", "complicada", "posible", "imposible",
        "importante", "necesario", "necesaria", "especial", "diferente", "igual", "común", "raro", "rara",
        "alegre", "triste", "enojado", "enojada", "cansado", "cansada", "enfermo", "enferma", "sano", "sana",
        "valiente", "cobarde", "simpático", "simpática", "antipático", "antipática", "generoso", "generosa", "egoísta",
        "trabajador", "trabajadora", "perezoso", "perezosa", "honesto", "honesta", "mentiroso", "mentirosa",
        "limpio", "limpia", "sucio", "sucia", "ordenado", "ordenada", "desordenado", "desordenada",
        "abierto", "abierta", "cerrado", "cerrada", "lleno", "llena", "vacío", "vacía"
    };
    static const std::vector<std::string> irregulares_verbos = {
        "ser", "ir", "haber", "estar", "tener", "hacer", "poder", "decir", "ver", "dar",
        "saber", "querer", "llegar", "pasar", "deber", "poner", "parecer", "quedar", "creer",
        "venir", "salir", "valer", "caber", "caer", "traer", "oír", "oler",
        "andar", "conducir", "traducir", "conocer", "reconocer", "agradecer", "ofrecer", "parecer", "pertenecer",
        "nacer", "obedecer", "enriquecer", "envejecer", "oscurecer", "permanecer", "establecer",
        "jugar", "pensar", "entender", "empezar", "comenzar", "perder", "preferir", "sentir",
        "dormir", "morir", "pedir", "servir", "repetir", "seguir", "conseguir", "perseguir",
        "vestir", "rendir", "elegir", "corregir", "medir", "reñir", "teñir", "freír", "reír", "sonreír",
        "mover", "llover", "volver", "resolver", "devolver", "envolver", "conmover", "promover",
        "soler", "doler", "satisfacer", "deshacer", "rehacer",
        "incluir", "construir", "destruir", "huir", "sustituir", "distinguir", "extinguir",
        "erguir", "adquirir", "inquirir",
        "roer", "raer", "yacer", "placer", "cocer", "torcer", "vencer", "mecer"
    };
    static const std::vector<std::string> comunes_adverbios = {
        "aquí", "allí", "ahí", "cerca", "lejos", "despacio", "rápido", "bien", "mal", "mucho",
        "poco", "nunca", "siempre", "también", "tampoco", "solo", "solamente", "inclusive",
        "adelante", "atrás", "arriba", "abajo", "dentro", "fuera", "encima", "debajo",
        "enfrente", "detrás", "alrededor", "dondequiera",
        "antes", "después", "luego", "pronto", "tarde", "temprano", "ayer", "hoy", "mañana",
        "anoche", "entonces", "todavía", "ya", "aún", "incluso", "jamás", "frecuentemente",
        "a menudo", "a veces", "raramente", "recién", "antiguamente", "últimamente", "mientras",
        "deprisa", "lentamente", "fácilmente", "difícilmente", "cuidadosamente", "especialmente",
        "así", "tal", "cómo", "dónde", "cuándo", "cuánto",
        "mejor", "peor", "regular", "más", "menos", "casi", "aproximadamente", "exactamente",
        "quizá", "quizás", "acaso", "seguramente", "probablemente", "efectivamente",
        "sí", "no", "ciertamente", "cierto", "también", "tampoco", "además", "asimismo"
    };
    static const std::vector<std::string> demonstrativos = {
        "este", "esta", "estos", "estas", "esto", "estos",
        "ese", "esa", "esos", "esas", "eso",
        "aquel", "aquella", "aquellos", "aquellas", "aquello"
    };
    static const std::vector<std::string> numerales = {
        "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve", "diez",
        "once", "doce", "trece", "catorce", "quince", "veinte", "cien", "mil", "primer", "tercer",
        "veintiuno", "veintidós", "veintitrés", "veinticuatro", "veinticinco",
        "veintiséis", "veintisiete", "veintiocho", "veintinueve",
        "treinta", "cuarenta", "cincuenta", "sesenta", "setenta", "ochenta", "noventa",
        "ciento", "doscientos", "trescientos", "cuatrocientos", "quinientos",
        "seiscientos", "setecientos", "ochocientos", "novecientos",
        "millón",
        "primero", "segundo", "tercero", "cuarto", "quinto", "sexto",
        "séptimo", "octavo", "noveno", "décimo",
        "undécimo", "duodécimo", "vigésimo", "trigésimo", "cuadragésimo",
        "quincuagésimo", "sexagésimo", "septuagésimo", "octogésimo", "nonagésimo"
    };
    static const std::vector<std::string> relativos = {
        "que", "quien", "quienes", "cuyo", "cuya", "cuyos", "cuyas",
        "el cual", "la cual", "lo cual",
        "los cuales", "las cuales",
        "cuanto", "cuanta", "cuantos", "cuantas"
    };
    static const std::vector<std::string> cuantificadores = {
        "mucho", "mucha", "muchos", "muchas", "poco", "poca", "pocos", "pocas",
        "varios", "varias", "todo", "toda", "todos", "todas", "algo", "nada",
        "bastante", "demasiado", "demasiada", "demasiados", "demasiadas",
        "más", "menos", "tanto", "tanta", "tantos", "tantas", "alguno", "alguna",
        "algunos", "algunas", "ningún", "ninguno", "ninguna", "cualquier", "cualquiera",
        "ambos", "ambas", "cada", "sendos", "sendas", "diversos", "diversas", "distintos",
        "suficiente", "suficientes", "demasiado", "demasiada", "demasiados", "demasiadas",
        "alguien", "nadie", "algo", "nada", "todo", "toda", "todos", "todas",
        "cada", "cualquiera", "quienquiera", "uno", "una", "unos", "unas",
        "numeroso", "numerosa", "numerosos", "numerosas",
        "escaso", "escasa", "escasos", "escasas", "distintas"
    };
    static const std::vector<std::string> preposiciones = {
        "a", "al", "del", "ante", "bajo", "con", "de", "en",
        "para", "por", "sin", "sobre", "tras", "durante", "mediante",
        "desde", "hasta", "hacia", "según", "contra", "entre",
        "excepto", "salvo", "so", "cabe", "versus", "vía", "pro"
    };
    static const std::vector<std::string> conjunciones = {
        "y", "e", "o", "u",
        "pero", "sino", "mas",
        "porque", "pues", "conque", "luego", "así",
        "si", "aunque", "ni", "que",
        "como", "cuando", "mientras", "siquiera"
    };
    static const std::vector<std::string> articulos = {
        "el", "la", "los", "las", "lo", "un", "una", "unos", "unas"
    };
    static const std::vector<std::string> pronombres = {
        "yo", "tú", "vos", "él", "ella", "ello", "nosotros", "nosotras",
        "vosotros", "vosotras", "ellos", "ellas", "usted", "ustedes",
        "me", "te", "se", "lo", "la", "le", "nos", "os"
    };
    static const std::vector<std::string> posesivos = {
        "mío", "mía", "míos", "mías", "tuyo", "tuya", "tuyos", "tuyas",
        "suyo", "suya", "suyos", "suyas", "nuestro", "nuestra", "nuestros", "nuestras",
        "vuestro", "vuestra", "vuestros", "vuestras"
    };
    static const std::vector<std::string> interrogativas = {
        "qué", "cuál", "cuáles", "cómo", "cuándo", "dónde", "adónde",
        "por qué", "para qué", "cuánto", "cuánta", "cuántos", "cuántas", "cuán"
        "quién", "quiénes", "cuánta"
    };

    // Sufijos
    static const std::vector<std::string> sufijos_sustantivo = {
        "ción", "sión", "xión", "dad", "tad", "eza", "ez",
        "ancia", "encia", "icia", "icie", "ismo", "aje", "ambre", "umbre",
        "tud", "dura", "anza", "ncia", "tor", "sor", "dor",
        "ista", "ería","ada", "miento", "mento", "tura", "or", "ud",
        "-ito", "ita", "illo", "illa", "ico", "ica",
        "uelo", "uela","-ón", "ona", "azo", "aza", "ote", "ota"
    };
    static const std::vector<std::string> sufijos_verbo = {
        "ar", "er", "ir", "ando", "iendo", "yendo",
        "ado", "ido", "aba", "abas", "ábamos", "aban",
        "ía", "ías", "íamos", "ían", "are", "ere", "ire",
        "aria", "ería", "iría", "aste", "iste", "asteis", "isteis",
        "aron", "ieron", "ó", "ió", "as", "a", "amos", "áis", "an",
        "es", "e", "emos", "éis", "en", "é", "aste", "ó", "amos",
        "asteis", "aron","í", "iste", "ió", "imos", "isteis", "ieron",
        "abais", "aban", "íais", "ían", "aré", "eré", "iré",
        "arás", "erás", "irás", "ará", "erá", "irá", "aremos", "eremos", "iremos",
        "aréis", "eréis", "iréis", "arán", "erán", "irán", "e", "es", "e",
        "emos", "éis", "en", "a", "as", "a", "amos", "áis", "an", "ra",
        "ras", "ra", "ramos", "rais", "ran", "se", "ses", "se", "semos", "seis",
        "sen","are", "ares", "are", "áremos", "areis", "aren"
    };
    static const std::vector<std::string> sufijos_adjetivo = {
        "oso", "osa", "ivo", "iva", "ble", "nte", "ante", "ente",
        "al", "il","ario", "aria", "ero", "era", "dora", "dero", "dera",
        "ativo", "itiva", "esco", "esca", "uno", "una", "izo", "iza",
        "torio", "toria", "ano", "ana", "és", "esa", "ino", "ina", "ense",
        "eño", "eña", "iento", "ienta", "udo", "uda", "iente", "ico", "ica",
        "ístico", "ística", "ático", "ática", "able", "ible", "ón", "ona"
    };
    static const std::vector<std::string> sufijos_adverbio = { "mente" };

    // ==================== FUNCIONES AUXILIARES ====================
    bool endsWith(const std::string& palabra, const std::string& sufijo) {
        if (sufijo.size() > palabra.size()) return false;
        return palabra.compare(palabra.size() - sufijo.size(), sufijo.size(), sufijo) == 0;
    }

    static bool endsWithAny(const std::string& palabra, const std::vector<std::string>& lista) {
        for (const auto& suf : lista) if (endsWith(palabra, suf)) return true;
        return false;
    }

    static bool isInList(const std::string& palabra, const std::vector<std::string>& lista) {
        return std::find(lista.begin(), lista.end(), palabra) != lista.end();
    }

    // Implementaciones de las funciones públicas

    bool isCommonWord(const std::string& word, TipoPalabra& outTag, float& outConf) {
        if (isInList(word, comunes_sustantivos)) { outTag = TipoPalabra::SUST; outConf = 0.95f; return true; }
        if (isInList(word, comunes_adjetivos))   { outTag = TipoPalabra::ADJT; outConf = 0.95f; return true; }
        if (isInList(word, irregulares_verbos))  { outTag = TipoPalabra::VERB; outConf = 0.95f; return true; }
        if (isInList(word, comunes_adverbios))   { outTag = TipoPalabra::ADV;  outConf = 0.95f; return true; }
        if (isInList(word, demonstrativos))      { outTag = TipoPalabra::DEMS; outConf = 0.95f; return true; }
        if (isInList(word, numerales))           { outTag = TipoPalabra::NUM;  outConf = 0.95f; return true; }
        if (isInList(word, relativos))           { outTag = TipoPalabra::RELT; outConf = 0.95f; return true; }
        if (isInList(word, cuantificadores))     { outTag = TipoPalabra::CUANT;outConf = 0.95f; return true; }
        if (isInList(word, articulos))           { outTag = TipoPalabra::ART;  outConf = 0.99f; return true; }
        if (isInList(word, preposiciones))       { outTag = TipoPalabra::PREP; outConf = 0.98f; return true; }
        if (isInList(word, conjunciones))        { outTag = TipoPalabra::CONJ; outConf = 0.97f; return true; }
        if (isInList(word, interrogativas))      { outTag = TipoPalabra::PREG; outConf = 0.96f; return true; }
        if (isInList(word, pronombres))          { outTag = TipoPalabra::PRON; outConf = 0.97f; return true; }
        if (isInList(word, posesivos))           { outTag = TipoPalabra::PRON; outConf = 0.5f; return true; }
        return false;
    }

    float validateTag(const std::string& palabra, TipoPalabra tag) {
        switch (tag) {
            case TipoPalabra::SUST: return endsWithAny(palabra, sufijos_sustantivo) || isPlural(palabra) ? 0.70f : 0.15f;
            case TipoPalabra::VERB: return endsWithAny(palabra, sufijos_verbo) || isInList(palabra, irregulares_verbos) ? 0.75f : 0.15f;
            case TipoPalabra::ADJT: return endsWithAny(palabra, sufijos_adjetivo) ? 0.70f : 0.15f;
            case TipoPalabra::ADV:  return endsWithAny(palabra, sufijos_adverbio) || isInList(palabra, comunes_adverbios) ? 0.80f : 0.10f;
            case TipoPalabra::PREG: return isInterrogative(palabra) ? 0.96f : 0.0f;
            case TipoPalabra::DEMS: return isDemonstrative(palabra) ? 0.90f : 0.10f;
            case TipoPalabra::NUM:  return isNumeral(palabra) ? 0.85f : 0.10f;
            case TipoPalabra::RELT: return isRelativePronoun(palabra) ? 0.90f : 0.10f;
            case TipoPalabra::CUANT:return isQuantifier(palabra) ? 0.80f : 0.10f;
            default: return 0.0f;
        }
    }

    bool isPlural(const std::string& palabra) {
        static const std::vector<std::string> excepciones = {"crisis", "martes", "paraguas", "lunes", "miércoles", "jueves", "viernes", "sábado", "domingo", "tórax", "fórceps", "virus", "atlas", "mes", "país", "cactus"};
        if (isInList(palabra, excepciones)) return false;
        if (palabra.size() < 2) return false;
        return palabra.back() == 's';
    }

    Genero detectGender(const std::string& palabra) {
        static const std::vector<std::string> ex_fem = {"mapa", "día", "problema", "sistema", "idioma", "clima", "programa", "tema"};
        static const std::vector<std::string> ex_masc = {"mano", "radio", "moto", "foto", "modelo", "imagen"};
        if (isInList(palabra, ex_fem)) return Genero::MASC;
        if (isInList(palabra, ex_masc)) return Genero::FEME;
        static const std::vector<std::string> term_fem = {"a", "ción", "sión", "dad", "tad", "umbre", "eza", "iz", "triz"};
        static const std::vector<std::string> term_masc = {"o", "l", "n", "r", "s", "ma", "ta", "pa", "aje"};
        if (endsWithAny(palabra, term_fem)) return Genero::FEME;
        if (endsWithAny(palabra, term_masc)) return Genero::MASC;
        return Genero::NEUT;
    }

    Tiempo detectTense(const std::string& palabra) {
        if (endsWith(palabra, "ó") || endsWith(palabra, "ió") || endsWith(palabra, "aba") ||
            endsWith(palabra, "ía") || endsWith(palabra, "aste") || endsWith(palabra, "iste") ||
            endsWith(palabra, "aron") || endsWith(palabra, "ieron") || endsWith(palabra, "ado") ||
            endsWith(palabra, "ido")) return Tiempo::PASS;
        if (endsWith(palabra, "aré") || endsWith(palabra, "eré") || endsWith(palabra, "iré") ||
            endsWith(palabra, "ará") || endsWith(palabra, "erá") || endsWith(palabra, "irá"))
            return Tiempo::FUTR;
        return Tiempo::PRES;
    }

    Persona detectPerson(const std::string& palabra) {
        if (endsWith(palabra, "o") && !endsWith(palabra, "mos") && !endsWith(palabra, "ís") && !endsWith(palabra, "n"))
            return Persona::PRIM;
        if (endsWith(palabra, "as") || endsWith(palabra, "es") || endsWith(palabra, "ís"))
            return Persona::SEGU;
        if (endsWith(palabra, "a") || endsWith(palabra, "e"))
            return Persona::TERC;
        if (endsWith(palabra, "mos")) return Persona::PRIM;
        if (endsWith(palabra, "n")) return Persona::TERC;
        return Persona::NIN;
    }

    Grado detectAdjectiveDegree(const std::string& palabra) {
        if (endsWith(palabra, "ísimo") || endsWith(palabra, "érrimo") ||
            endsWith(palabra, "ísima") || endsWith(palabra, "érrima"))
            return Grado::SUPERLA;
        if (endsWith(palabra, "or") && palabra.size() > 3)
            return Grado::COMPARA;
        return Grado::POSIT;
    }

    TipoPalabra guessInitialTag(const std::string& palabra) {
        if (isArticle(palabra)) return TipoPalabra::ART;
        if (isPreposition(palabra)) return TipoPalabra::PREP;
        if (isConjunction(palabra)) return TipoPalabra::CONJ;
        if (isInterrogative(palabra)) return TipoPalabra::PREG;
        TipoPalabra commTag; float dummy;
        if (isCommonWord(palabra, commTag, dummy)) return commTag;
        if (endsWithAny(palabra, sufijos_verbo)) return TipoPalabra::VERB;
        if (endsWithAny(palabra, sufijos_adjetivo)) return TipoPalabra::ADJT;
        if (endsWithAny(palabra, sufijos_adverbio)) return TipoPalabra::ADV;
        if (endsWithAny(palabra, sufijos_sustantivo)) return TipoPalabra::SUST;
        return TipoPalabra::INDEFINIDO;
    }

    float getSuffixProb(const std::string& palabra, TipoPalabra tag) {
        if (tag == TipoPalabra::ART && isArticle(palabra)) return 0.99f;
        if (tag == TipoPalabra::PREP && isPreposition(palabra)) return 0.98f;
        if (tag == TipoPalabra::CONJ && isConjunction(palabra)) return 0.97f;
        if (tag == TipoPalabra::PREG && isInterrogative(palabra)) return 0.96f;
        TipoPalabra commTag; float commonConf;
        if (isCommonWord(palabra, commTag, commonConf) && commTag == tag) return commonConf;
        float base = (palabra.size() < 3) ? 0.01f : 0.05f;
        switch (tag) {
            case TipoPalabra::SUST: return endsWithAny(palabra, sufijos_sustantivo) ? 0.35f : base;
            case TipoPalabra::VERB: return endsWithAny(palabra, sufijos_verbo) ? 0.40f : base;
            case TipoPalabra::ADJT: return endsWithAny(palabra, sufijos_adjetivo) ? 0.35f : base;
            case TipoPalabra::ADV:  return endsWithAny(palabra, sufijos_adverbio) ? 0.70f : base;
            case TipoPalabra::DEMS: return isDemonstrative(palabra) ? 0.90f : base;
            case TipoPalabra::NUM:  return isNumeral(palabra) ? 0.85f : base;
            case TipoPalabra::RELT: return isRelativePronoun(palabra) ? 0.85f : base;
            case TipoPalabra::CUANT:return isQuantifier(palabra) ? 0.80f : base;
            default: return base;
        }
    }

    // Funciones de reconocimiento directo
    bool isArticle(const std::string& palabra) { return isInList(palabra, articulos); }
    bool isPreposition(const std::string& palabra) { return isInList(palabra, preposiciones); }
    bool isConjunction(const std::string& palabra) { return isInList(palabra, conjunciones); }
    bool isInterrogative(const std::string& palabra) { return isInList(palabra, interrogativas); }
    bool isDemonstrative(const std::string& palabra) { return isInList(palabra, demonstrativos); }
    bool isNumeral(const std::string& palabra) { return isInList(palabra, numerales) || std::regex_match(palabra, std::regex("\\d+")); }
    bool isRelativePronoun(const std::string& palabra) { return isInList(palabra, relativos); }
    bool isQuantifier(const std::string& palabra) { return isInList(palabra, cuantificadores); }

} // namespace morphology
