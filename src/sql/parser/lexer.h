#include "state.h"

namespace toydb::sql {

        enum Token {
            // A number literal
            Number,
            // A string literal
            String,
            // A textual identifier
            Ident,
            // The period symbol .
            Period,
            // The equals symbol =
            Equals,
            // The greater-than symbol >
            GreaterThan,
            // The less-than symbol <
            LessThan,
            // The addition symbol +
            Plus,
            // The subtraction symbol -
            Minus,
            // The multiplication symbol *
            Asterisk,
            // The division symbol /
            Slash,
            // THe exponentiation symbol ^
            Caret,
            // The modulo symbol %
            Percent,
            // THe factorial or not symbol !
            Exclamation,
            // THe query parameter market ?
            Question,
            // An opening parenthesis
            OpenParen,
            // A closing parenthesis
            CloseParen,
            // An expression separator ,
            Comma,
        };
        typedef bool (*match_predicate_fun) (char);
    typedef Status<Token> (*match_single_char_token_fun) (char);

        class Lexer {
        public:
            Lexer(std::string& data): data_(data) {};

            // Consumes any whitespace characters
            void consume_whitespace();

            // Grabs the next character if it matches the predicate function
            Status<char> next_if(match_predicate_fun f);

            // Grabs the next single-character token if the tokenizer function returns one
            Status<Token> next_if_token(match_single_char_token_fun f);

            // Grabs the next characters that match the predicate, as a string
            Status<std::string> next_while(match_predicate_fun f);

            // Scans the input for the next token if any, ignoring leading whitespace
            Status<Token> scan();

            // Scans the input for the next ident token, if any
            Status<Token> scan_ident();

            // Scans the input for the next number token, if any
            Status<Token> scan_number();

            // Scans the input for the next string literal, if any
            Status<Token> scan_string();

            // Scans the input for the next symbol token, if any
            Status<Token> scan_symbol();


        private:
            std::string data_;
        };

};
