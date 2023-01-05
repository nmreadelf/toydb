//
// Created by elf on 5/1/23.
//
#include <gtest/gtest.h>
#include "kv.h"

using ::testing::InitGoogleTest;
using ::testing::Test;

namespace toydb {
TEST(StoreTest, Delete) {
    Store s;
        std::string key = "a";
        std::string value = "1";
        s.Set(key, value);
        std::string t;
        auto st = s.Get(key, &t);
        EXPECT_TRUE(st.ok());
        EXPECT_EQ(value, t);

        s.Delete(key);
        st = s.Get(key, &t);
        EXPECT_TRUE(IsNotFound(st));
        st = s.Get("b", &t);
        EXPECT_TRUE(IsNotFound(st));
}
    TEST(StoreTest, Get) {
        Store s;
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

        Store s;
        for (int i = 0; i < values.size(); i++) {
            s.Set(key, values[i]);
            std::string v;
            auto st = s.Get(key, &v);
            EXPECT_TRUE(st.ok());
            EXPECT_EQ(v, values[i]);
        }
    }
}

int main(int argc, char** argv) {
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}