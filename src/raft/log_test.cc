#include "raft/log.h"
#include <gtest/gtest.h>

using ::testing::InitGoogleTest;
using ::testing::Test;

namespace toydb::raft {
std::pair<std::shared_ptr<Log>, std::shared_ptr<KvStore>> buildLog() {
  std::shared_ptr<KvStore> s(new KvStore);
  auto res = Log::Build(s);
  EXPECT_TRUE(res.first.ok());
  std::shared_ptr<Log> log(res.second);
  return std::make_pair(std::shared_ptr<Log>(res.second), s);
}

std::shared_ptr<Log> buildLog(std::shared_ptr<KvStore> &s) {
  auto res = Log::Build(s);
  EXPECT_TRUE(res.first.ok());
  std::shared_ptr<Log> log(res.second);
  return log;
}

TEST(RaftTest, Build) {
  auto r = buildLog();
  auto log = r.first;
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
  auto r = buildLog();
  auto log = r.first;
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
    // Append none command
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
  auto r = buildLog();
  auto log = r.first;
  auto s = r.second;
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
  auto log2 = buildLog(s);
  for (const auto i : idsx) {
    auto res = log2->Get(i);
    EXPECT_TRUE(res.first.ok());
    EXPECT_EQ(res.second->command(), es[i - 1]->command());
    EXPECT_EQ(res.second->term(), es[i - 1]->term());
  }
}

TEST(RaftTest, ApplyTest) {
  Entry e1;
  std::string e1_cmd = "1";
  e1.set_term(1);
  e1.set_command(e1_cmd);
  Entry e2;
  e2.set_term(2);
  Entry e3;
  std::string e3_cmd = "3";
  e3.set_term(2);
  e3.set_command(e3_cmd);
  std::vector<uint64_t> idsx;
  std::vector<Entry *> es{&e1, &e2, &e3};
  {

    auto r = buildLog();
    auto s = r.second;
    {
      auto log = r.first;
      for (const auto e : es) {
        auto res = log->Append(*e);
        EXPECT_TRUE(res.first.ok());
        idsx.push_back(res.second);
      }
      {
        auto res = log->Commit(3);
        EXPECT_TRUE(std::get<0>(res).ok());
        EXPECT_EQ(3, std::get<1>(res));
      }
      TestState ts;
      {
        auto res = log->Apply(&ts);
        EXPECT_TRUE(std::get<0>(res).ok());
        EXPECT_EQ(1, std::get<1>(res));
        EXPECT_EQ("1", std::get<2>(res));
        EXPECT_EQ(std::vector<std::string>{e1_cmd}, ts.List());
      }
      {
        auto res = log->GetApplied();
        EXPECT_EQ(1, res.first);
        EXPECT_EQ(1, res.second);
      }
      {
        auto res = log->Apply(&ts);
        EXPECT_TRUE(std::get<0>(res).ok());
        EXPECT_EQ(2, std::get<1>(res));
        EXPECT_EQ("", std::get<2>(res));
        EXPECT_EQ(std::vector<std::string>{e1_cmd}, ts.List());
      }
      {
        auto res = log->GetApplied();
        EXPECT_EQ(2, res.first);
        EXPECT_EQ(2, res.second);
      }
      {
        Entry e;
        auto res = log->Apply(&ts);
        EXPECT_TRUE(std::get<0>(res).ok());
        EXPECT_EQ(3, std::get<1>(res));
        EXPECT_EQ("3", std::get<2>(res));
        std::vector<std::string> ee{e1_cmd, e3_cmd};
        EXPECT_EQ(ee, ts.List());
      }
      {
        auto res = log->GetApplied();
        EXPECT_EQ(3, res.first);
        EXPECT_EQ(2, res.second);
      }
      {
        auto res = log->Apply(&ts);
        EXPECT_TRUE(std::get<0>(res).ok());
        EXPECT_EQ(0, std::get<1>(res));
        EXPECT_EQ("", std::get<2>(res));
      }
      {
        auto res = log->GetApplied();
        EXPECT_EQ(3, res.first);
        EXPECT_EQ(2, res.second);
      }
    }
    // The last applied entry should be persisted, and also used for last
    // committed
    {
      auto ll = buildLog(s);
      {
        auto res = ll->GetLast();
        EXPECT_EQ(3, res.first);
        EXPECT_EQ(2, res.second);
      }
      {
        auto res = ll->GetCommitted();
        EXPECT_EQ(3, res.first);
        EXPECT_EQ(2, res.second);
      }
      {
        auto res = ll->GetApplied();
        EXPECT_EQ(3, res.first);
        EXPECT_EQ(2, res.second);
      }
    }
  }
  {

    auto r = buildLog();
    auto s = r.second;
    auto log = r.first;

    for (const auto e : es) {
      auto res = log->Append(*e);
      EXPECT_TRUE(res.first.ok());
      idsx.push_back(res.second);
    }
    {
      auto res = log->Commit(1);
      EXPECT_TRUE(std::get<0>(res).ok());
    }
    TestState st;
    {
      auto res = log->Apply(&st);
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(1, std::get<1>(res));
      EXPECT_EQ("1", std::get<2>(res));
      auto rr = log->GetApplied();
      EXPECT_EQ(1, rr.first);
      EXPECT_EQ(1, rr.second);
      EXPECT_EQ(std::vector<std::string>{e1_cmd}, st.List());
    }
    {
      auto res = log->Apply(&st);
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(0, std::get<1>(res));
      EXPECT_EQ("", std::get<2>(res));
      auto rr = log->GetApplied();
      EXPECT_EQ(1, rr.first);
      EXPECT_EQ(1, rr.second);
      EXPECT_EQ(std::vector<std::string>{e1_cmd}, st.List());
    }
  }
}

TEST(RaftTest, CommitTest) {
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  Entry e3;
  e3.set_term(2);
  e3.set_command("3");
  std::vector<uint64_t> idsx;
  std::vector<Entry *> es{&e1, &e2, &e3};
  {
    auto r = buildLog();
    auto s = r.second;
    {
      auto log = r.first;
      for (const auto e : es) {
        auto res = log->Append(*e);
        EXPECT_TRUE(res.first.ok());
        idsx.push_back(res.second);
      }
      {
        auto res = log->Commit(3);
        EXPECT_TRUE(std::get<0>(res).ok());
        EXPECT_EQ(3, std::get<1>(res));
      }
      auto res = log->GetCommitted();
      EXPECT_EQ(3, res.first);
      EXPECT_EQ(2, res.second);
    }
    auto log2 = buildLog(s);
    {
      auto res = log2->GetCommitted();
      EXPECT_EQ(0, res.first);
      EXPECT_EQ(0, res.second);
    }
  }
  // Commit beyond
  {
    auto r = buildLog();
    auto s = r.second;
    auto ll = r.first;

    for (const auto e : es) {
      auto res = ll->Append(*e);
      EXPECT_TRUE(res.first.ok());
    }
    {
      auto res = ll->Commit(4);
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(3, std::get<1>(res));
    }
    {
      auto res = ll->GetCommitted();
      EXPECT_EQ(3, res.first);
      EXPECT_EQ(2, res.second);
    }
  }
  // Commit partial
  {
    auto r = buildLog();
    auto s = r.second;
    auto ll = r.first;

    for (const auto e : es) {
      auto res = ll->Append(*e);
      EXPECT_TRUE(res.first.ok());
    }
    {
      auto res = ll->Commit(2);
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(2, std::get<1>(res));
    }
    {
      auto res = ll->GetCommitted();
      EXPECT_EQ(2, res.first);
      EXPECT_EQ(2, res.second);
    }
  }
  // Commit reduce
  {
    auto r = buildLog();
    auto s = r.second;
    auto ll = r.first;

    for (const auto e : es) {
      auto res = ll->Append(*e);
      EXPECT_TRUE(res.first.ok());
    }
    {
      auto res = ll->Commit(2);
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(2, std::get<1>(res));
    }
    {
      auto res = ll->GetCommitted();
      EXPECT_EQ(2, res.first);
      EXPECT_EQ(2, res.second);
    }
    {
      auto res = ll->Commit(1);
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(2, std::get<1>(res));
    }
    {
      auto res = ll->GetCommitted();
      EXPECT_EQ(2, res.first);
      EXPECT_EQ(2, res.second);
    }
  }
}

TEST(RaftTest, GetTest) {
  std::shared_ptr<KvStore> s = std::make_shared<KvStore>();
  auto log = buildLog(s);
  {
    auto res = log->Get(1);
    EXPECT_TRUE(!res.first.ok());
    EXPECT_EQ(res.second, nullptr);
  }
  Entry e;
  e.set_term(3);
  e.set_command("1");
  {
    auto res = log->Append(e);
    EXPECT_TRUE(res.first.ok());
  }
  {
    auto res = log->Get(1);
    EXPECT_TRUE(res.first.ok());
    EXPECT_EQ(e.term(), res.second->term());
    EXPECT_EQ(e.command(), res.second->command());
  }
  {
    auto res = log->Get(2);
    EXPECT_TRUE(!res.first.ok());
    EXPECT_EQ(res.second, nullptr);
  }
}

TEST(RaftTest, HasTest) {
  std::shared_ptr<KvStore> s = std::make_shared<KvStore>();
  auto log = buildLog(s);
  {
    auto res = log->Get(1);
    EXPECT_TRUE(!res.first.ok());
    EXPECT_EQ(res.second, nullptr);
  }
  Entry e;
  e.set_term(2);
  e.set_command("1");
  {
    auto res = log->Append(e);
    EXPECT_TRUE(res.first.ok());
  }
  {
    EXPECT_TRUE(log->Has(1, 2));
    EXPECT_TRUE(log->Has(0, 0));
    EXPECT_FALSE(log->Has(0, 1));
    EXPECT_FALSE(log->Has(1, 0));
    EXPECT_FALSE(log->Has(1, 3));
    EXPECT_FALSE(log->Has(2, 0));
    EXPECT_FALSE(log->Has(2, 1));
  }
}

TEST(RaftTest, RangeTest) {
  std::shared_ptr<KvStore> s = std::make_shared<KvStore>();
  auto log = buildLog(s);
  {
    auto res = log->Get(1);
    EXPECT_TRUE(!res.first.ok());
    EXPECT_EQ(res.second, nullptr);
  }
  {
    Entry e;
    e.set_term(1);
    std::vector<std::string> cmds{"1", "2", "3"};
    for (const auto &c : cmds) {
      e.set_command(c);
      auto res = log->Append(e);
      EXPECT_TRUE(res.first.ok());
    }
    log->Range(0);
    for (const auto &c : cmds) {
      e.set_command(c);
      auto res = log->Append(e);
      EXPECT_TRUE(res.first.ok());
    }
  }
  {
    EXPECT_TRUE(log->Has(1, 2));
    EXPECT_TRUE(log->Has(0, 0));
    EXPECT_FALSE(log->Has(0, 1));
    EXPECT_FALSE(log->Has(1, 0));
    EXPECT_FALSE(log->Has(1, 3));
    EXPECT_FALSE(log->Has(2, 0));
    EXPECT_FALSE(log->Has(2, 1));
  }
}

TEST(RaftTest, LoadSaveTermTest) {
  std::shared_ptr<KvStore> s = std::make_shared<KvStore>();
  {
    auto log = buildLog(s);
    {
      auto res = log->LoadTerm();
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(0, std::get<1>(res));
      EXPECT_EQ("", std::get<2>(res));
    }
    std::string a("a");
    auto res = log->SaveTerm(1, a);
    EXPECT_TRUE(res.ok());
  }
  {
    auto log = buildLog(s);
    {
      auto res = log->LoadTerm();
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(1, std::get<1>(res));
      EXPECT_EQ("a", std::get<2>(res));
    }
    std::string a("c");
    auto res = log->SaveTerm(3, a);
    EXPECT_TRUE(res.ok());
  }
  {
    auto log = buildLog(s);
    {
      auto res = log->LoadTerm();
      EXPECT_TRUE(std::get<0>(res).ok());
      EXPECT_EQ(3, std::get<1>(res));
      EXPECT_EQ("c", std::get<2>(res));
    }
    std::string a;
    auto res = log->SaveTerm(0, a);
    EXPECT_TRUE(res.ok());
  }
  {
    auto log = buildLog(s);
    auto res = log->LoadTerm();
    EXPECT_TRUE(std::get<0>(res).ok());
    EXPECT_EQ(0, std::get<1>(res));
    EXPECT_EQ("", std::get<2>(res));
  }
}

} // namespace toydb::raft

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
