//
// Created by elf on 3/20/23.
//

#include "sql/parser/lexer.h"
#include <gtest/gtest.h>

using ::testing::InitGoogleTest;
using ::testing::Test;

namespace toydb::sql {

TEST(SqlLexerTest, LiteralString) {
  std::string input(
      R"(A 'literal string with ''single'' and "double" quotes inside 😀'.)");
  std::vector<Token> v{
      Token{Ident, "A"},
      Token{String,
            R"(literal string with ''single'' and "double" quotes inside 😀)"},
      Token{Period},
  };

  Lexer c(input);

  for (const auto &i : v) {
    auto res = c.NextToken();
    ASSERT_TRUE(std::get<0>(res) == i);
  }
}

TEST(SqlLexerTest, Literal_number) {
  std::string input("0 1 3.14 293. -2.718 3.14e3 2.718E-2");
  std::vector<Token> v{
      Token{Number, "0"},
      Token{Number, "1"},
      Token{Number, "3.14"},
      Token{Number, "293."},
      Token{Minus},
      Token{Number, "2.718"},
      Token{Number, "3.14e3"},
      Token{Number, "2.718E-2"},
  };
  Lexer c(input);

  for (const auto &i : v) {
    auto res = c.NextToken();
    ASSERT_TRUE(std::get<0>(res) == i);
  }
}

TEST(SqlLexerTest, Select) {
  std::string input(
      R"(SELECT artist.name, album.name, EXTRACT(YEAR FROM NOW()) - album.release_year AS age
        FROM artist INNER JOIN album ON album.artist_id = artist.id
        WHERE album.genre != 'country' AND album.release_year >= 1980
        ORDER BY artist.name ASC, age DESC)");
  std::vector<Token> v{
      Token{Ident, "SELECT"},
      Token{Ident, "artist"},
      Token{Period},
      Token{Ident, "name"},
      Token{Comma},
      Token{Ident, "album"},
      Token{Period},
      Token{Ident, "name"},
      Token{Comma},
      Token{Ident, "EXTRACT"},
      Token{OpenParen},
      Token{Ident, "YEAR"},
      Token{Ident, "FROM"},
      Token{Ident, "NOW"},
      Token{OpenParen},
      Token{CloseParen},
      Token{CloseParen},
      Token{Minus},
      Token{Ident, "album"},
      Token{Period},
      Token{Ident, "release_year"},
      Token{Ident, "AS"},
      Token{Ident, "age"},
      Token{Ident, "FROM"},
      Token{Ident, "artist"},
      Token{Ident, "INNER"},
      Token{Ident, "JOIN"},
      Token{Ident, "album"},
      Token{Ident, "ON"},
      Token{Ident, "album"},
      Token{Period},
      Token{Ident, "artist_id"},
      Token{Equals},
      Token{Ident, "artist"},
      Token{Period},
      Token{Ident, "id"},
      Token{Ident, "WHERE"},
      Token{Ident, "album"},
      Token{Period},
      Token{Ident, "genre"},
      Token{Exclamation},
      Token{Equals},
      Token{String, "country"},
      Token{Ident, "AND"},
      Token{Ident, "album"},
      Token{Period},
      Token{Ident, "release_year"},
      Token{GreaterThan},
      Token{Equals},
      Token{Number, "1980"},
      Token{Ident, "ORDER"},
      Token{Ident, "BY"},
      Token{Ident, "artist"},
      Token{Period},
      Token{Ident, "name"},
      Token{Ident, "ASC"},
      Token{Comma},
      Token{Ident, "age"},
      Token{Ident, "DESC"},
  };
  Lexer c(input);

  for (const auto &i : v) {
    auto res = c.NextToken();
    ASSERT_TRUE(std::get<0>(res) == i);
  }
}

} // namespace toydb::sql

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
