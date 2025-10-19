#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <random>
#include <memory>
#include <variant>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <unordered_map>

using namespace std;

// All possible token types in Sprout
enum class TokenType {
    // Keywords
    INT, STR, FLOAT, ARRAY,
    PRINT, INPUT, IF, ELSE, JUMP, BREAK, RANDOM, LENGTH,
    READ,

    // Identifiers / literals
    IDENT, NUMBER, STRING,

    // Symbols
    PLUS, MINUS, STAR, SLASH,
    EQ,             // "=" (assignment)
    EQEQ,           // "==" (equality)
    NE,             // "!=" (not equal)
    LT, GT,         // "<", ">"
    LE, GE,         // "<=", ">="
    LPAREN, RPAREN, // ( )
    LBRACE, RBRACE, // { }
    LBRACKET, RBRACKET, // [ ]
    COMMA, SEMICOLON,
    COLON,          // ":" (needed for print : and input :)
    UNKNOWN,        // fallback for unexpected chars

    // End of file
    END_OF_FILE
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
        while (true) {
            while (isspace(peek())) advance();
            // handle // comments
            if (peek() == '/' && (pos + 1) < src.size() && src[pos + 1] == '/') {
                while (peek() != '\n' && peek() != '\0') advance();
                continue;
            }
            break;
        }
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
            if (word == "int")    return makeToken(TokenType::INT, word);
            if (word == "str")    return makeToken(TokenType::STR, word);
            if (word == "float")  return makeToken(TokenType::FLOAT, word);
            if (word == "array")  return makeToken(TokenType::ARRAY, word);
            if (word == "print")  return makeToken(TokenType::PRINT, word);
            if (word == "input")  return makeToken(TokenType::INPUT, word);
            if (word == "if")     return makeToken(TokenType::IF, word);
            if (word == "else")   return makeToken(TokenType::ELSE, word);
            if (word == "jump")   return makeToken(TokenType::JUMP, word);
            if (word == "break")  return makeToken(TokenType::BREAK, word);
            if (word == "random") return makeToken(TokenType::RANDOM, word);
            if (word == "read")   return makeToken(TokenType::READ, word);
            if (word == "len") return makeToken(TokenType::LENGTH, word);
            return makeToken(TokenType::IDENT, word);
        }

        // Numbers
        if (isdigit(c)) {
            string num;
            while (isdigit(peek())) num += advance();
            // Support optional fractional part: digits '.' digits
            if (peek() == '.' && (pos + 1) < src.size() && isdigit(src[pos + 1])) {
                num += advance(); // consume '.'
                while (isdigit(peek())) num += advance();
            }
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

        // Two-character and single-character operators/symbols
        // Handle multi-char first: ==, !=, <=, >=
        if (c == '=') {
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::EQEQ, "=="); }
            return makeToken(TokenType::EQ, "=");
        }
        if (c == '!') {
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::NE, "!="); }
            return makeToken(TokenType::UNKNOWN, "!");
        }
        if (c == '<') {
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::LE, "<="); }
            return makeToken(TokenType::LT, "<");
        }
        if (c == '>') {
            advance();
            if (peek() == '=') { advance(); return makeToken(TokenType::GE, ">="); }
            return makeToken(TokenType::GT, ">");
        }

        // Single-character tokens
        switch (advance()) {
            case '+': return makeToken(TokenType::PLUS, "+");
            case '-': return makeToken(TokenType::MINUS, "-");
            case '*': return makeToken(TokenType::STAR, "*");
            case '/': return makeToken(TokenType::SLASH, "/");
            case '(': return makeToken(TokenType::LPAREN, "(");
            case ')': return makeToken(TokenType::RPAREN, ")");
            case '{': return makeToken(TokenType::LBRACE, "{");
            case '}': return makeToken(TokenType::RBRACE, "}");
            case '[': return makeToken(TokenType::LBRACKET, "[");
            case ']': return makeToken(TokenType::RBRACKET, "]");
            case ':': return makeToken(TokenType::COLON, ":");
            case ',': return makeToken(TokenType::COMMA, ",");
            case ';': return makeToken(TokenType::SEMICOLON, ";");
        }

        return makeToken(TokenType::UNKNOWN, string(1, c));
    }

    Token makeToken(TokenType type, const string& text) {
        return Token{type, text, line};
    }
};

// Forward declarations
struct Expr;
struct Stmt;

using ExprPtr = shared_ptr<Expr>;
using StmtPtr = shared_ptr<Stmt>;

/* =====================
   EXPRESSIONS (things that produce values)
   ===================== */
struct Expr {
    virtual ~Expr() = default;
};

struct NumberExpr : Expr {
    double value;
    NumberExpr(double v) : value(v) {}
};

struct StringExpr : Expr {
    string value;
    StringExpr(string v) : value(move(v)) {}
};

struct ArrayExpr : Expr {
   vector<ExprPtr> values;
    ArrayExpr(vector<ExprPtr> v) : values(move(v)) {}
};

struct ArrayAccessExpr : Expr {
    string arrayName;
    ExprPtr index;
    ArrayAccessExpr(string name, ExprPtr idx) : arrayName(move(name)), index(move(idx)) {}
};

struct VarExpr : Expr {
    string name;
    VarExpr(string n) : name(move(n)) {}
};

struct BinaryExpr : Expr {
    string op;       // "+", "-", "*", "/", "="
    ExprPtr left;
    ExprPtr right;
    BinaryExpr(string o, ExprPtr l, ExprPtr r)
        : op(move(o)), left(move(l)), right(move(r)) {}
};

/* =====================
   STATEMENTS (things that *do* stuff)
   ===================== */
struct Stmt {
    virtual ~Stmt() = default;
    int line; // Add line number tracking
};

// Update all statement structs to include line number in constructor
struct DeclStmt : Stmt {
    string type;
    string name;
    ExprPtr init;
    DeclStmt(string t, string n, ExprPtr e, int l)
        : type(move(t)), name(move(n)), init(move(e)) { line = l; }
};

struct ArrayDeclStmt : Stmt {
    string name;
    vector<ExprPtr> values = vector<ExprPtr>();
    ExprPtr init;
    ArrayDeclStmt(string n, ExprPtr e, int l)
        : name(move(n)), init(move(e)) { line = l; }
};

struct AssignStmt : Stmt {
    string name;
    ExprPtr expr;
    AssignStmt(string n, ExprPtr e, int l)
        : name(move(n)), expr(move(e)) { line = l; }
};

struct PrintStmt : Stmt {
    ExprPtr expr;
    PrintStmt(ExprPtr e, int l) : expr(move(e)) { line = l; }
};

struct InputStmt : Stmt {
    string name;
    string question;
    InputStmt(string n, string q, int l)
        : name(move(n)), question(move(q)) { line = l; }
};

struct RandomStmt : Stmt {
    string name;
    int min;
    int max;
    RandomStmt(string n, int m, int M, int l)
        : name(move(n)), min(m), max(M) { line = l; }
};

struct IfStmt : Stmt {
    vector<pair<ExprPtr, vector<StmtPtr>>> branches;
    IfStmt(int l) { line = l; }
};

struct JumpStmt : Stmt {
    int jumpTo;
    JumpStmt(int j, int l) : jumpTo(j) { line = l; }
};

struct BreakStmt : Stmt {
    BreakStmt(int l) { line = l; }
};

struct ReadStmt : Stmt {
    string varName;
    string fileName;
    ReadStmt(string v, string f, int l) : varName(move(v)), fileName(move(f)) { line = l; }
};

struct LengthStmt : Stmt {
    string varName;
    string ArrayName;
    LengthStmt(string v, string a, int l) : varName(move(v)), ArrayName(move(a)) { line = l; }
};

class Parser {
    vector<Token> tokens;
    size_t pos;

public:
    Parser(vector<Token> t) : tokens(move(t)), pos(0) {}

    vector<StmtPtr> parseProgram() {
        vector<StmtPtr> stmts;
        while (!isAtEnd()) {
            size_t before = pos;
            StmtPtr s = parseStmt();
            if (!s) break;
            stmts.push_back(s);
            // Safety: ensure progress to avoid infinite loop
            if (pos == before) break;
        }
        // consume a single EOF if present
        if (!tokens.empty() && pos < tokens.size() && tokens[pos].type == TokenType::END_OF_FILE) {
            ++pos;
        }
        return stmts;
    }

private:
    // === Utility ===
    Token& peek() {
        static Token eofToken{TokenType::END_OF_FILE, "", 0};
        if (pos >= tokens.size()) return eofToken;
        return tokens[pos];
    }
    bool match(TokenType type) {
        if (check(type)) { ++pos; return true; }
        return false;
    }
    Token& advance() {
        static Token eofToken{TokenType::END_OF_FILE, "", 0};
        if (pos >= tokens.size()) return eofToken;
        return tokens[pos++];
    }
    bool check(TokenType type) {
        if (isAtEnd()) return type == TokenType::END_OF_FILE;
        return peek().type == type;
    }
    bool isAtEnd() {
        return pos >= tokens.size() || tokens[pos].type == TokenType::END_OF_FILE;
    }

    // === Parsing ===
    StmtPtr parseStmt() {
        if (match(TokenType::INT))    return parseDecl("int");
        if (match(TokenType::STR))    return parseDecl("str");
        if (match(TokenType::FLOAT))  return parseDecl("float");
        if (match(TokenType::ARRAY))  return parseArray();
        if (match(TokenType::PRINT))  return parsePrint();
        if (match(TokenType::INPUT))  return parseInput();
        if (match(TokenType::IF))     return parseIf();
        if (match(TokenType::JUMP))   return parseJump();
        if (match(TokenType::BREAK))  return parseBreak();
        if (match(TokenType::RANDOM)) return parseRandom();
        if (match(TokenType::READ))   return parseRead();
        if (match(TokenType::LENGTH)) return parseLength();

        // Otherwise → assignment
        return parseAssign();
    }

    StmtPtr parseJump() {
        Token jumpTok = tokens[pos-1]; // Get the jump token for line number
        match(TokenType::COLON);       // Expect colon after 'jump'
        Token tok = advance();

        // Verify we have a number token for the line number
        if (tok.type != TokenType::NUMBER) {
            throw runtime_error("Expected line number after 'jump :'");
        }

        return make_shared<JumpStmt>(stoi(tok.text), jumpTok.line);
    }

    StmtPtr parseBreak() {
        Token breakTok = tokens[pos-1]; // Get the break token for line number
        return make_shared<BreakStmt>(breakTok.line);
    }

    StmtPtr parseDecl(string type) {
        Token typeTok = tokens[pos-1]; // Get type token for line number
        Token name = advance();
        match(TokenType::EQ);
        ExprPtr expr = parseExpr();
        return make_shared<DeclStmt>(type, name.text, expr, typeTok.line);
    }

    StmtPtr parseArray() {
        Token arrayTok = tokens[pos-1]; // Get array token for line number
        Token name = advance();
        match(TokenType::EQ);
        vector<ExprPtr> values;
        if (match(TokenType::LPAREN)) {
            while (!check(TokenType::RPAREN)) {
                values.push_back(parseExpr());
                if (!check(TokenType::COMMA)) break;
                advance();
            }
            match(TokenType::RPAREN);
        }
        return make_shared<DeclStmt>("array", name.text, make_shared<ArrayExpr>(values), arrayTok.line);
    }

    StmtPtr parseAssign() {
        Token nameTok = advance(); // Get name token for line number
        match(TokenType::EQ);
        ExprPtr expr = parseExpr();
        return make_shared<AssignStmt>(nameTok.text, expr, nameTok.line);
    }

    StmtPtr parsePrint() {
        Token printTok = tokens[pos-1]; // Get print token for line number
        match(TokenType::COLON);
        ExprPtr expr = parseExpr();
        return make_shared<PrintStmt>(expr, printTok.line);
    }

    StmtPtr parseInput() {
        Token inputTok = tokens[pos-1]; // Get input token for line number
        match(TokenType::COLON);
        Token name = advance();
        match(TokenType::COMMA);
        Token question = advance();
        return make_shared<InputStmt>(name.text, question.text, inputTok.line);
    }

    StmtPtr parseRead() {
        Token readTok = tokens[pos-1];
        match(TokenType::COLON);
        Token varName = advance();
        match(TokenType::COMMA);
        Token fileName = advance();
        return make_shared<ReadStmt>(varName.text, fileName.text, readTok.line);
    }

    StmtPtr parseRandom() {
        Token randomTok = tokens[pos-1];
        match(TokenType::COLON);
        Token name = advance();
        match(TokenType::COMMA);
        Token min = advance();
        match(TokenType::COMMA);
        Token max = advance();
        return make_shared<RandomStmt>(name.text, stoi(min.text), stoi(max.text), randomTok.line);
    }

    StmtPtr parseLength() {
        Token lengthTok = tokens[pos-1];
        match(TokenType::COLON);
        Token varName = advance();
        match(TokenType::COMMA);
        Token arrayName = advance();
        return make_shared<LengthStmt>(varName.text, arrayName.text, lengthTok.line);
    }

    StmtPtr parseIf() {
        Token ifTok = tokens[pos-1]; // Get if token for line number
        auto node = make_shared<IfStmt>(ifTok.line);
        vector<pair<ExprPtr, vector<StmtPtr>>> branches;

        // if (...)
        match(TokenType::LPAREN);
        ExprPtr cond = parseExpr();
        match(TokenType::RPAREN);
        match(TokenType::COLON);
        vector<StmtPtr> body;
        while (!check(TokenType::SEMICOLON) &&
               !check(TokenType::ELSE) &&
               !check(TokenType::END_OF_FILE)) {
            body.push_back(parseStmt());
               }
        branches.push_back({cond, body});

        // consume optional ';' that terminates the if-body
        match(TokenType::SEMICOLON);

        // optional else
        if (match(TokenType::ELSE)) {
            match(TokenType::COLON);
            vector<StmtPtr> elseBody;
            while (!check(TokenType::SEMICOLON) && !check(TokenType::END_OF_FILE)) {
                elseBody.push_back(parseStmt());
            }
            branches.push_back({nullptr, elseBody});

            // consume optional ';' that terminates the else-body
            match(TokenType::SEMICOLON);
        }

        node->branches = move(branches);
        return node;
    }

    // === Expressions ===
    ExprPtr parseExpr() {
        return parseEquality();
    }

    // equality -> comparison ( (== | !=) comparison )*
    ExprPtr parseEquality() {
        ExprPtr left = parseComparison();
        while (match(TokenType::EQEQ) || match(TokenType::NE)) {
            Token op = tokens[pos - 1];
            ExprPtr right = parseComparison();
            left = make_shared<BinaryExpr>(op.text, left, right);
        }
        return left;
    }

    // comparison -> addSub ( (< | > | <= | >=) addSub )*
    ExprPtr parseComparison() {
        ExprPtr left = parseAddSub();
        while (match(TokenType::LT) || match(TokenType::GT) || match(TokenType::LE) || match(TokenType::GE)) {
            Token op = tokens[pos - 1];
            ExprPtr right = parseAddSub();
            left = make_shared<BinaryExpr>(op.text, left, right);
        }
        return left;
    }

    ExprPtr parseAddSub() {
        ExprPtr left = parseTerm();
        while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
            Token op = tokens[pos - 1];
            ExprPtr right = parseTerm();
            left = make_shared<BinaryExpr>(op.text, left, right);
        }
        return left;
    }

    ExprPtr parseTerm() {
        ExprPtr left = parseFactor();
        while (match(TokenType::STAR) || match(TokenType::SLASH)) {
            Token op = tokens[pos - 1];
            ExprPtr right = parseFactor();
            left = make_shared<BinaryExpr>(op.text, left, right);
        }
        return left;
    }

    ExprPtr parseFactor() {
        Token tok = advance();
        if (tok.type == TokenType::NUMBER)
            return make_shared<NumberExpr>(stod(tok.text));
        if (tok.type == TokenType::STRING)
            return make_shared<StringExpr>(tok.text);
        // Do not treat 'float' keyword as a literal
        // if (tok.type == TokenType::FLOAT) ...  // removed
        if (tok.type == TokenType::IDENT) {
            string name = tok.text;
            // Check for array access: identifier[index]
            if (match(TokenType::LBRACKET)) {
                ExprPtr index = parseExpr();
                match(TokenType::RBRACKET);
                return make_shared<ArrayAccessExpr>(name, index);
            }
            return make_shared<VarExpr>(name);
        }

        // parenthesized expr
        if (tok.type == TokenType::LPAREN) {
            ExprPtr expr = parseExpr();
            match(TokenType::RPAREN);
            return expr;
        }

        return nullptr;
    }
};
/* =====================
   INTERPRETER
   - walks the AST
   - keeps variables in memory
   ===================== */

// Update the variable storage to support arrays
class Interpreter {
    unordered_map<string, variant<double, string, vector<variant<double, string>>>> variables;

public:
    void run(const vector<StmtPtr>& program) {
        // Build line number to statement index mapping
        unordered_map<int, size_t> lineToIndex;
        for (size_t i = 0; i < program.size(); i++) {
            lineToIndex[program[i]->line] = i;
        }

        size_t currentStmt = 0;
        while (currentStmt < program.size()) {
            try {
                currentStmt = exec(program[currentStmt], program, lineToIndex, currentStmt);
            } catch (const JumpException& e) {
                // Handle jump by finding the target line
                auto it = lineToIndex.find(e.targetLine);
                if (it != lineToIndex.end()) {
                    currentStmt = it->second;
                } else {
                    cerr << "Error: Cannot jump to line " << e.targetLine << " - line not found\n";
                    break;
                }
            } catch (const BreakException&) {
                break; // Exit the program
            }
        }
    }

private:
    // Exception classes for control flow
    class JumpException : public exception {
    public:
        int targetLine;
        JumpException(int line) : targetLine(line) {}
    };

    class BreakException : public exception {};

    // Modified exec to return next statement index
    size_t exec(const StmtPtr& stmt, const vector<StmtPtr>& program,
                const unordered_map<int, size_t>& lineToIndex, size_t currentIndex) {
        if (auto d = dynamic_pointer_cast<DeclStmt>(stmt)) {
            execDecl(d);
            return currentIndex + 1;
        }
        else if (auto a = dynamic_pointer_cast<AssignStmt>(stmt)) {
            execAssign(a);
            return currentIndex + 1;
        }
        else if (auto p = dynamic_pointer_cast<PrintStmt>(stmt)) {
            execPrint(p);
            return currentIndex + 1;
        }
        else if (auto i = dynamic_pointer_cast<InputStmt>(stmt)) {
            execInput(i);
            return currentIndex + 1;
        }
        else if (auto ifs = dynamic_pointer_cast<IfStmt>(stmt)) {
            return execIf(ifs, program, lineToIndex, currentIndex);
        }
        else if (auto j = dynamic_pointer_cast<JumpStmt>(stmt)) {
            throw JumpException(j->jumpTo);
        }
        else if (auto b = dynamic_pointer_cast<BreakStmt>(stmt)) {
            throw BreakException();
        }
        else if (auto r = dynamic_pointer_cast<RandomStmt>(stmt)) {
            execRandom(r);
            return currentIndex + 1;
        }
        else if (auto rd = dynamic_pointer_cast<ReadStmt>(stmt)) {
            execRead(rd);
            return currentIndex + 1;
        }
        else if (auto l = dynamic_pointer_cast<LengthStmt>(stmt)) {
            execLength(l);
            return currentIndex + 1;
        }
        else {
            throw runtime_error("Unknown statement type");
        }
    }

    size_t execIf(const shared_ptr<IfStmt>& stmt, const vector<StmtPtr>& program,
                  const unordered_map<int, size_t>& lineToIndex, size_t currentIndex) {
        for (auto& branch : stmt->branches) {
            ExprPtr cond = branch.first;
            auto& body = branch.second;

            if (!cond) {
                // Else branch - execute all statements in else body WITHOUT advancing the global index
                for (auto& s : body) {
                    size_t ignore = exec(s, program, lineToIndex, currentIndex);
                    (void)ignore;
                }
                return currentIndex + 1;
            }

            auto condVal = eval(cond);
            bool truthy = false;
            if (holds_alternative<double>(condVal)) {
                truthy = (get<double>(condVal) != 0);
            } else if (holds_alternative<string>(condVal)) {
                truthy = !get<string>(condVal).empty();
            }

            if (truthy) {
                // Execute all statements in if body WITHOUT advancing the global index
                for (auto& s : body) {
                    size_t ignore = exec(s, program, lineToIndex, currentIndex);
                    (void)ignore;
                }
                return currentIndex + 1;
            }
        }
        return currentIndex + 1;
    }

    void execDecl(const shared_ptr<DeclStmt>& stmt) {
        auto val = eval(stmt->init);
        variables[stmt->name] = val;
    }

    void execAssign(const shared_ptr<AssignStmt>& stmt) {
        auto val = eval(stmt->expr);
        variables[stmt->name] = val;
    }

    void execPrint(const shared_ptr<PrintStmt>& stmt) {
        auto val = eval(stmt->expr);
        if (holds_alternative<double>(val)) {
            cout << get<double>(val) << "\n";
        } else if (holds_alternative<string>(val)) {
            cout << get<string>(val) << "\n";
        } else if (holds_alternative<vector<variant<double, string>>>(val)) {
            cout << "[";
            auto& arr = get<vector<variant<double, string>>>(val);
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) cout << ", ";
                if (holds_alternative<double>(arr[i])) {
                    cout << get<double>(arr[i]);
                } else {
                    cout << get<string>(arr[i]);
                }
            }
            cout << "]\n";
        }
    }

    void execInput(const shared_ptr<InputStmt>& stmt) {
        cout << stmt->question << " ";
        string userInput;
        getline(cin, userInput);

        // If var was declared as number → convert
        if (variables.find(stmt->name) != variables.end() &&
            holds_alternative<double>(variables[stmt->name])) {
            variables[stmt->name] = stod(userInput);
        } else {
            variables[stmt->name] = userInput;
        }
    }

    void execRandom(const shared_ptr<RandomStmt>& stmt) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(stmt->min, stmt->max);
        double val = dist(gen);
        variables[stmt->name] = val;
    }

    void execRead(const shared_ptr<ReadStmt>& stmt) {
        ifstream file(stmt->fileName);
        if (!file.is_open()) {
            throw runtime_error("Cannot open file " + stmt->fileName);
        }
        vector<string> lines;
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        vector<variant<double, string>> arr;
        for (auto& l : lines) {
            arr.push_back(l);
        }
        variables[stmt->varName] = arr;
    }

    void execLength(const shared_ptr<LengthStmt>& stmt) {
        // Look up the array variable
        auto it = variables.find(stmt->ArrayName);
        if (it == variables.end()) {
            throw runtime_error("Undefined variable: " + stmt->ArrayName);
        }

        // Make sure it's actually an array
        if (!holds_alternative<vector<variant<double, string>>>(it->second)) {
            throw runtime_error("Variable " + stmt->ArrayName + " is not an array");
        }

        // Get the actual size of the array
        auto& array = get<vector<variant<double, string>>>(it->second);
        double len = array.size();
        variables[stmt->varName] = len;
    }

    // === Expression Evaluation ===
    variant<double, string, vector<variant<double, string>>> eval(const ExprPtr& expr) {
        if (auto n = dynamic_pointer_cast<NumberExpr>(expr)) {
            return n->value;
        }
        if (auto s = dynamic_pointer_cast<StringExpr>(expr)) {
            return s->value;
        }
        if (auto a = dynamic_pointer_cast<ArrayExpr>(expr)) {
            vector<variant<double, string>> values;
            for (auto& v : a->values) {
                auto result = eval(v);
                if (holds_alternative<double>(result)) {
                    values.push_back(get<double>(result));
                } else if (holds_alternative<string>(result)) {
                    values.push_back(get<string>(result));
                } else if (holds_alternative<vector<variant<double, string>>>(result)) {
                    // For nested arrays, we could either flatten them or convert to string
                    // For simplicity, let's convert nested arrays to string representation
                    values.push_back(toString(result));
                }
            }
            return values;
        }
        if (auto aa = dynamic_pointer_cast<ArrayAccessExpr>(expr)) {
            // Find the array variable
            auto it = variables.find(aa->arrayName);
            if (it == variables.end()) {
                throw runtime_error("Undefined array: " + aa->arrayName);
            }
            
            // Make sure it's actually an array
            if (!holds_alternative<vector<variant<double, string>>>(it->second)) {
                throw runtime_error("Variable " + aa->arrayName + " is not an array");
            }
            
            // Evaluate the index
            auto indexVal = eval(aa->index);
            if (!holds_alternative<double>(indexVal)) {
                throw runtime_error("Array index must be a number");
            }
            
            int index = (int)get<double>(indexVal);
            auto& array = get<vector<variant<double, string>>>(it->second);
            
            // Check bounds
            if (index < 0 || index >= (int)array.size()) {
                throw runtime_error("Array index out of bounds: " + to_string(index));
            }
            
            // Return the element (convert single element back to our 3-type variant)
            if (holds_alternative<double>(array[index])) {
                return get<double>(array[index]);
            } else {
                return get<string>(array[index]);
            }
        }
        if (auto v = dynamic_pointer_cast<VarExpr>(expr)) {
            // STRICT LOOKUP: do not default-initialize missing variables
            auto it = variables.find(v->name);
            if (it == variables.end()) {
                throw runtime_error("Undefined variable: " + v->name);
            }
            return it->second;
        }
        if (auto b = dynamic_pointer_cast<BinaryExpr>(expr)) {
            auto left = eval(b->left);
            auto right = eval(b->right);

            // arithmetic on numbers
            if (holds_alternative<double>(left) && holds_alternative<double>(right)) {
                double l = get<double>(left);
                double r = get<double>(right);
                if (b->op == "+") return l + r;
                if (b->op == "-") return l - r;
                if (b->op == "*") return l * r;
                if (b->op == "/") return l / r;
                if (b->op == "<") return (l < r) ? 1.0 : 0.0;
                if (b->op == ">") return (l > r) ? 1.0 : 0.0;
                if (b->op == "==") return (l == r) ? 1.0 : 0.0;
                if (b->op == "<=") return (l <= r) ? 1.0 : 0.0;
                if (b->op == ">=") return (l >= r) ? 1.0 : 0.0;
                if (b->op == "!=") return (l != r) ? 1.0 : 0.0;
            }

            // string concatenation with +
            if (b->op == "+") {
                string l = toString(left);
                string r = toString(right);
                return l + r;
            }
        }
        return 0.0; // fallback
    }

    string toString(const variant<double, string, vector<variant<double, string>>>& val) {
        if (holds_alternative<double>(val)) {
            double num = get<double>(val);
            if (num == (int)num) {
                return to_string((int)num); // no decimals if whole number
            }
            return to_string(num);
        } else if (holds_alternative<string>(val)) {
            return get<string>(val);
        } else if (holds_alternative<vector<variant<double, string>>>(val)) {
            string result = "[";
            auto& arr = get<vector<variant<double, string>>>(val);
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) result += ", ";
                if (holds_alternative<double>(arr[i])) {
                    double num = get<double>(arr[i]);
                    if (num == (int)num) {
                        result += to_string((int)num);
                    } else {
                        result += to_string(num);
                    }
                } else {
                    result += get<string>(arr[i]);
                }
            }
            result += "]";
            return result;
        }
        return "";
    }
};

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;

    // Collect candidate paths from CLI args; prefer @file:... entries
    vector<string> candidates;
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a.rfind("@file:", 0) == 0) {
            candidates.push_back(a.substr(6));
        } else if (!a.empty() && a[0] != '@') {
            candidates.push_back(a);
        }
        // ignore markers like @thisFile
    }
    // Default fallbacks if nothing provided or none valid
    if (candidates.empty()) {
        candidates.push_back("test.spt");
        candidates.push_back("sprout/Sprout_C++/test.spt");
    }

    auto resolveUpwards = [](const string& rel) -> string {
        namespace fs = std::filesystem;
        // Try relative to current working directory and walk up
        fs::path cwd = fs::current_path();
        for (int i = 0; i < 10; ++i) {
            fs::path candidate = cwd / rel;
            if (fs::exists(candidate)) return candidate.string();
            if (!cwd.has_parent_path()) break;
            cwd = cwd.parent_path();
        }
        return rel;
    };

    ifstream in;
    string openedPath;
    for (const auto& cand : candidates) {
        string path = resolveUpwards(cand);
        in.open(path);
        if (in) { openedPath = path; break; }
        in.clear();
    }

    if (!in) {
        cerr << "Failed to open file. Tried:";
        for (const auto& c : candidates) cerr << " " << c;
        cerr << "\n";
        return 1;
    }

    stringstream buffer;
    buffer << in.rdbuf();
    string source = buffer.str();

    Lexer lexer(source);
    vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    vector<StmtPtr> program = parser.parseProgram();

    Interpreter interpreter;
    interpreter.run(program);

    return 0;
}