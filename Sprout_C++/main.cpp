#include <iostream>
#include <string>
#include <vector>
#include <cctype>
using namespace std;

// All possible token types in Sprout
enum class TokenType {
    INT, STR, PRINT, INPUT, IF, ELSE,   // keywords
    IDENT, NUMBER, STRING,              // identifiers and literals
    PLUS, MINUS, STAR, SLASH, EQUAL,    // operators
    LPAREN, RPAREN, LBRACE, RBRACE,     // symbols ( )
    COLON, SEMICOLON,                   // : and ;
    END_OF_FILE,
    UNKNOWN
};

// A single token: type + text + (optional) line number
struct Token {
    TokenType type;
    string text;
    int line;
};

// Our Lexer class
class Lexer {
    string src;  // source code
    size_t pos;  // current index
    int line;    // current line number

public:
    Lexer(const string& input) : src(input), pos(0), line(1) {}

    // Get all tokens until END_OF_FILE
    vector<Token> tokenize() {
        vector<Token> tokens;
        Token tok = nextToken();
        while (tok.type != TokenType::END_OF_FILE) {
            tokens.push_back(tok);
            tok = nextToken();
        }
        tokens.push_back(tok); // push EOF
        return tokens;
    }

private:
    // Helper: peek at current character
    char peek() {
        if (pos >= src.size()) return '\0'; // end of text
        return src[pos];
    }

    // Helper: advance to next character
    char advance() {
        char c = peek();
        pos++;
        if (c == '\n') line++;
        return c;
    }

    // Skip spaces and comments
    void skipWhitespace() {
        while (isspace(peek())) advance();
    }

    // Core: get the next token
    Token nextToken() {
        skipWhitespace();

        char c = peek();
        if (c == '\0') return makeToken(TokenType::END_OF_FILE, "");

        // Identifiers or keywords
        if (isalpha(c)) {
            string word;
            while (isalnum(peek())) word += advance();
            if (word == "int")   return makeToken(TokenType::INT, word);
            if (word == "str")   return makeToken(TokenType::STR, word);
            if (word == "print") return makeToken(TokenType::PRINT, word);
            if (word == "input") return makeToken(TokenType::INPUT, word);
            if (word == "if")    return makeToken(TokenType::IF, word);
            if (word == "else")  return makeToken(TokenType::ELSE, word);
            return makeToken(TokenType::IDENT, word);
        }

        // Numbers
        if (isdigit(c)) {
            string num;
            while (isdigit(peek())) num += advance();
            return makeToken(TokenType::NUMBER, num);
        }

        // Strings "..."
        if (c == '"') {
            advance(); // skip "
            string value;
            while (peek() != '"' && peek() != '\0') value += advance();
            advance(); // skip closing "
            return makeToken(TokenType::STRING, value);
        }

        // Operators and symbols
        switch (advance()) {
            case '+': return makeToken(TokenType::PLUS, "+");
            case '-': return makeToken(TokenType::MINUS, "-");
            case '*': return makeToken(TokenType::STAR, "*");
            case '/': return makeToken(TokenType::SLASH, "/");
            case '=': return makeToken(TokenType::EQUAL, "=");
            case '(': return makeToken(TokenType::LPAREN, "(");
            case ')': return makeToken(TokenType::RPAREN, ")");
            case '{': return makeToken(TokenType::LBRACE, "{");
            case '}': return makeToken(TokenType::RBRACE, "}");
            case ':': return makeToken(TokenType::COLON, ":");
            case ';': return makeToken(TokenType::SEMICOLON, ";");
        }

        return makeToken(TokenType::UNKNOWN, string(1, c));
    }

    Token makeToken(TokenType type, const string& text) {
        return Token{type, text, line};
    }
};
