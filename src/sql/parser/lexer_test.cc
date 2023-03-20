//
// Created by elf on 3/20/23.
//

#include "sql/parser/lexer.h"
#include <gtest/gtest.h>

using ::testing::InitGoogleTest;
using ::testing::Test;

namespace toydb::sql {

    TEST(SqlLexerTest, LiteralString) {
        std::string input(R"(A 'literal string with ''single'' and "double" quotes inside 😀'.)");
    }

    TEST(SqlLexerTest, Literal_number) {
        std::string input("0 1 3.14 293. -2.718 3.14e3 2.718E-2");
    }

    TEST(SqlLexerTest, Select) {
        std::string input(R"(SELECT artist.name, album.name, EXTRACT(YEAR FROM NOW()) - album.release_year AS age
        FROM artist INNER JOIN album ON album.artist_id = artist.id
        WHERE album.genre != 'country' AND album.release_year >= 1980
        ORDER BY artist.name ASC, age DESC)");
    }

}

int main(int argc, char **argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
