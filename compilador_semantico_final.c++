#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_set>
#include <unordered_map>

// Tipos de tokens
enum class TokenType {
    KEYWORD,       
    IDENTIFIER,    
    NUMBER,        
    OPERATOR,      
    WHITESPACE,   
    UNKNOWN        
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    explicit Lexer(const std::string& input) : input(input), position(0) {
        keywords = {"if", "for", "while", "return", "int", "float", "else"};
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (position < input.size()) {
            char currentChar = input[position];

            if (std::isspace(currentChar)) {
                tokens.push_back(Token{TokenType::WHITESPACE, std::string(1, currentChar)});
                position++;
            } else if (std::isalpha(currentChar)) {
                std::string identifier = parseIdentifier();
                if (keywords.find(identifier) != keywords.end()) {
                    tokens.push_back(Token{TokenType::KEYWORD, identifier});
                } else {
                    tokens.push_back(Token{TokenType::IDENTIFIER, identifier});
                }
            } else if (std::isdigit(currentChar)) {
                tokens.push_back(Token{TokenType::NUMBER, parseNumber()});
            } else if (isOperator(currentChar)) {
                tokens.push_back(Token{TokenType::OPERATOR, std::string(1, currentChar)});
                position++;
            } else {
                tokens.push_back(Token{TokenType::UNKNOWN, std::string(1, currentChar)});
                position++;
            }
        }
        return tokens;
    }

    const std::unordered_set<std::string>& getKeywords() const {
        return keywords;
    }

private:
    std::string input;
    size_t position;
    std::unordered_set<std::string> keywords;

    std::string parseIdentifier() {
        size_t start = position;
        while (position < input.size() && std::isalpha(input[position])) {
            position++;
        }
        return input.substr(start, position - start);
    }

    std::string parseNumber() {
        size_t start = position;
        while (position < input.size() && std::isdigit(input[position])) {
            position++;
        }
        return input.substr(start, position - start);
    }

    bool isOperator(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || 
               c == '&' || c == '|' || c == '!' || c == '<' || c == '>';
    }
}; // Aquí añadimos el punto y coma faltante

// Función de ayuda para imprimir tokens
void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::string type;
        switch (token.type) {
            case TokenType::KEYWORD: type = "KEYWORD"; break;
            case TokenType::IDENTIFIER: type = "IDENTIFIER"; break;
            case TokenType::NUMBER: type = "NUMBER"; break;
            case TokenType::OPERATOR: type = "OPERATOR"; break;
            case TokenType::WHITESPACE: type = "WHITESPACE"; break;
            case TokenType::UNKNOWN: type = "UNKNOWN"; break;
        }
        std::cout << "Type: " << type << ", Value: " << token.value << "\n";
    }
}

// Analizador Sintáctico
void syntaxAnalysis(const std::vector<Token>& tokens, const std::unordered_set<std::string>& keywords) {
    bool hasError = false;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];

        // Verificación de identificadores no válidos
        if (token.type == TokenType::IDENTIFIER && keywords.find(token.value) == keywords.end()) {
            // Verificar si es parte de una declaración o una operación válida
            if (i > 0 && (tokens[i - 1].type == TokenType::KEYWORD || tokens[i - 1].type == TokenType::OPERATOR || tokens[i - 1].type == TokenType::WHITESPACE)) {
                // No es un identificador inválido si está correctamente declarado o usado en una expresión
                continue;
            }
            std::cout << "Error de sintaxis: Identificador no valido '" << token.value << "'.\n";
            hasError = true;
            continue;
        }

        // Estructura `if`
        if (token.type == TokenType::KEYWORD && token.value == "if") {
            size_t j = i + 1;
            while (j < tokens.size() && tokens[j].type == TokenType::WHITESPACE) j++;

            if (j >= tokens.size() || tokens[j].value != "(") {
                std::cout << "Error de sintaxis: falta '(' despues de 'if'.\n";
                hasError = true;
            } else {
                size_t k = j + 1;
                while (k < tokens.size() && tokens[k].value != ")") k++;
                if (k >= tokens.size() || tokens[k].value != ")") {
                    std::cout << "Error de sintaxis: falta ')' en la condicion de 'if'.\n";
                    hasError = true;
                } else if (k + 1 >= tokens.size() || tokens[k + 1].value != "{") {
                    std::cout << "Error de sintaxis: falta '{' despues de 'if'.\n";
                    hasError = true;
                } else {
                    std::cout << "Estructura 'if' valida.\n";
                    i = k + 1;
                }
            }
        }
    }

    if (hasError) {
        std::cout << "Analisis sintactico detecto errores.\n";
    } else {
        std::cout << "Analisis sintactico completado sin errores.\n";
    }
}

// Analizador Semántico
void semanticAnalysis(const std::vector<Token>& tokens) {
    std::unordered_map<std::string, std::string> symbolTable; // Tabla de símbolos
    bool hasError = false;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];

        // Declaraciones de variables
        if (token.type == TokenType::KEYWORD && (token.value == "int" || token.value == "float")) {
            if (i + 1 < tokens.size()) {
                size_t j = i + 1;
                while (j < tokens.size() && tokens[j].type == TokenType::WHITESPACE) j++;
                
                if (j < tokens.size() && tokens[j].type == TokenType::IDENTIFIER) {
                    std::string varName = tokens[j].value;

                    if (symbolTable.find(varName) != symbolTable.end()) {
                        std::cout << "Error semantico: La variable '" << varName << "' ya esta declarada.\n";
                        hasError = true;
                    } else {
                        symbolTable[varName] = token.value;
                        std::cout << "Variable declarada: " << varName << " de tipo " << token.value << "\n";
                    }
                    j++;

                    // Verificar si hay una inicialización
                    while (j < tokens.size() && tokens[j].type == TokenType::WHITESPACE) j++;
                    if (j < tokens.size() && tokens[j].type == TokenType::OPERATOR && tokens[j].value == "=") {
                        j++; // Avanzar el índice para saltar el operador y el valor
                        while (j < tokens.size() && (tokens[j].type == TokenType::NUMBER || tokens[j].type == TokenType::WHITESPACE)) {
                            j++;
                        }
                    }
                    i = j - 1; // Ajustar el índice principal
                } else {
                    std::cout << "Error semantico: Falta el nombre de la variable despues de " << token.value << "\n";
                    hasError = true;
                }
            }
        }

        // Uso de variables
        else if (token.type == TokenType::IDENTIFIER) {
            if (symbolTable.find(token.value) == symbolTable.end()) {
                std::cout << "Error semantico: La variable '" << token.value << "' no esta declarada.\n";
                hasError = true;
            }
        }
    }

    if (hasError) {
        std::cout << "Analisis semantico detecto errores.\n";
    } else {
        std::cout << "Analisis semantico completado sin errores.\n";
    }
}

// Función principal
int main() {
    while (true) {
        std::string choice;
        std::cout << "Que tipo de analisis desea realizar? (lexico/sintactico/semantico/salir): ";
        std::cin >> choice;
        std::cin.ignore();

        if (choice == "salir") {
            std::cout << "Saliendo del programa.\n";
            break;
        }

        std::string code;
        std::cout << "Introduce el codigo a analizar: ";
        std::getline(std::cin, code);

        Lexer lexer(code);
        std::vector<Token> tokens = lexer.tokenize();

        if (choice == "lexico") {
            printTokens(tokens);
        } else if (choice == "sintactico") {
            syntaxAnalysis(tokens, lexer.getKeywords());
        } else if (choice == "semantico") {
            semanticAnalysis(tokens);
        } else {
            std::cout << "Opcion no valida.\n";
        }

        std::cout << "\nAnalisis completado.\n";
    }

    return 0;
}
