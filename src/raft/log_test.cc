#include "raft/log.cc"
#include <gtest/gtest.h>

using ::testing::InitGoogleTest;
using ::testing::Test;

namespace toydb::raft {
std::shared_ptr<Log> buildLog() {

  std::shared_ptr<KvStore> s(new KvStore);
  auto res = Log::Build(s);
  EXPECT_TRUE(res.first.ok());
  std::shared_ptr<Log> log(res.second);
  return log;
}
TEST(RaftTest, Build) {
  auto log = buildLog();
  {
    auto res = log->GetLast();
    EXPECT_EQ(res.first, 0);
    EXPECT_EQ(res.second, 0);
  }
  {
    auto res = log->GetCommitted();
    EXPECT_EQ(res.first, 0);
    EXPECT_EQ(res.second, 0);
  }
  {
    auto res = log->GetApplied();
    EXPECT_EQ(res.first, 0);
    EXPECT_EQ(res.second, 0);
  }
  {
    auto res = log->Get(1);
    EXPECT_TRUE(!res.first.ok());
  }
}
TEST(RaftTest, Append) {
  auto log = buildLog();
  {
    Entry e;
    e.set_command("1");
    e.set_term(3);
    auto res = log->Append(e);
    EXPECT_TRUE(res.first.ok());
    EXPECT_EQ(res.second, 1);
  }
  {
    auto res = log->Get(1);
    EXPECT_TRUE(res.first.ok());
    EXPECT_EQ(res.second->command(), "1");
    EXPECT_EQ(res.second->term(), 3);
  }
  {
    auto res = log->Get(2);
    EXPECT_TRUE(!res.first.ok());
  }
  {
    Entry e;
    e.set_term(3);
    auto res = log->Append(e);
    EXPECT_TRUE(res.first.ok());
    auto get_res = log->Get(res.second);
    EXPECT_TRUE(get_res.first.ok());
    EXPECT_EQ(get_res.second->command(), "");
    EXPECT_EQ(get_res.second->term(), 3);
  }
}
TEST(RaftTest, AppendPersistence) {
  std::shared_ptr<KvStore> s(new KvStore);
  Log *l;
  {
    auto res = Log::Build(s);
    EXPECT_TRUE(res.first.ok());
    l = res.second;
  }
  std::shared_ptr<Log> log(l);
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  std::vector<uint64_t> idsx;
  std::vector<Entry *> es{&e1, &e2, &e3};
  for (const auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.first.ok());
    idsx.push_back(res.second);
  }
  {
    auto res = Log::Build(s);
    EXPECT_TRUE(res.first.ok());
    l = res.second;
  }
  std::shared_ptr<Log> log2(l);
  for (const auto i : idsx) {
    auto res = log2->Get(i);
    EXPECT_TRUE(res.first.ok());
    EXPECT_EQ(res.second->command(), es[i - 1]->command());
    EXPECT_EQ(res.second->term(), es[i - 1]->term());
  }
}
} // namespace toydb::raft

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
