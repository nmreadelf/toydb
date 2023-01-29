//
// Created by elf on 5/1/23.
//
#include "kv.h"
#include <gtest/gtest.h>

using ::testing::InitGoogleTest;
using ::testing::Test;

namespace toydb {
TEST(StoreTest, Delete) {
  KvStore s;
  std::string key = "a";
  std::string value = "1";
  s.Set(key, value);
  std::string t;
  {
    auto st = s.Get(key, &t);
    EXPECT_TRUE(st.ok());
    EXPECT_EQ(value, t);
  }

  s.Delete(key);
  {

    auto st = s.Get(key, &t);
    EXPECT_FALSE(st.ok());
  }
  {
    auto st = s.Get("b", &t);
    EXPECT_FALSE(st.ok());
  }
}

TEST(StoreTest, Get) {
  KvStore s;
  std::string key = "a";
  std::string value = "1";
  s.Set(key, value);
  std::string t;
  auto st = s.Get(key, &t);
  EXPECT_TRUE(st.ok());
  EXPECT_EQ(value, t);
}

TEST(StoreTest, Set) {
  std::string key = "a";
  std::vector<std::string> values{"1", "2", "3"};

  KvStore s;
  for (const auto &value : values) {
    s.Set(key, value);
    std::string v;
    auto st = s.Get(key, &v);
    EXPECT_TRUE(st.ok());
    EXPECT_EQ(v, value);
  }
}

} // namespace toydb

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}