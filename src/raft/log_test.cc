#include "raft/log.h"
#include <gtest/gtest.h>

using ::testing::InitGoogleTest;
using ::testing::Test;

namespace toydb::raft {
std::pair<std::shared_ptr<Log>, std::shared_ptr<KvStore>> buildLog() {
  std::shared_ptr<KvStore> s(new KvStore);
  auto res = Log::Build(s);
  EXPECT_TRUE(res.ok());
  return std::make_pair(std::shared_ptr<Log>(res.value_), s);
}

std::shared_ptr<Log> buildLog(std::shared_ptr<KvStore> &s) {
  auto res = Log::Build(s);
  EXPECT_TRUE(res.ok());
  std::shared_ptr<Log> log(res.value_);
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
    EXPECT_TRUE(!res.ok());
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
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(res.value_, 1);
  }
  {
    auto res = log->Get(1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(res.value_->command(), "1");
    EXPECT_EQ(res.value_->term(), 3);
  }
  {
    auto res = log->Get(2);
    EXPECT_TRUE(!res.ok());
  }
  {
    // Append none command
    Entry e;
    e.set_term(3);
    auto res = log->Append(e);
    EXPECT_TRUE(res.ok());
    auto get_res = log->Get(res.value_);
    EXPECT_TRUE(get_res.ok());
    EXPECT_EQ(get_res.value_->command(), "");
    EXPECT_EQ(get_res.value_->term(), 3);
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
    EXPECT_TRUE(res.ok());
    idsx.push_back(res.value_);
  }
  auto log2 = buildLog(s);
  for (const auto i : idsx) {
    auto res = log2->Get(i);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(res.value_->command(), es[i - 1]->command());
    EXPECT_EQ(res.value_->term(), es[i - 1]->term());
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
        EXPECT_TRUE(res.ok());
        idsx.push_back(res.value_);
      }
      {
        auto res = log->Commit(3);
        EXPECT_TRUE(res.ok());
        EXPECT_EQ(3, res.value_);
      }
      TestState ts;
      {
        auto res = log->Apply(&ts);
        EXPECT_TRUE(res.ok());
        EXPECT_EQ(1, std::get<0>(res.value_));
        EXPECT_EQ("1", std::get<1>(res.value_));
        EXPECT_EQ(std::vector<std::string>{e1_cmd}, ts.List());
      }
      {
        auto res = log->GetApplied();
        EXPECT_EQ(1, res.first);
        EXPECT_EQ(1, res.second);
      }
      {
        auto res = log->Apply(&ts);
        EXPECT_TRUE(res.ok());
        EXPECT_EQ(2, std::get<0>(res.value_));
        EXPECT_EQ("", std::get<1>(res.value_));
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
        EXPECT_TRUE(res.ok());
        EXPECT_EQ(3, std::get<0>(res.value_));
        EXPECT_EQ("3", std::get<1>(res.value_));
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
        EXPECT_TRUE(res.ok());
        EXPECT_EQ(0, std::get<0>(res.value_));
        EXPECT_EQ("", std::get<1>(res.value_));
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
      EXPECT_TRUE(res.ok());
      idsx.push_back(res.value_);
    }
    {
      auto res = log->Commit(1);
      EXPECT_TRUE(res.ok());
    }
    TestState st;
    {
      auto res = log->Apply(&st);
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(1, std::get<0>(res.value_));
      EXPECT_EQ("1", std::get<1>(res.value_));
      auto rr = log->GetApplied();
      EXPECT_EQ(1, rr.first);
      EXPECT_EQ(1, rr.second);
      EXPECT_EQ(std::vector<std::string>{e1_cmd}, st.List());
    }
    {
      auto res = log->Apply(&st);
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(0, std::get<0>(res.value_));
      EXPECT_EQ("", std::get<1>(res.value_));
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
        EXPECT_TRUE(res.ok());
        idsx.push_back(res.value_);
      }
      {
        auto res = log->Commit(3);
        EXPECT_TRUE(res.ok());
        EXPECT_EQ(3, res.value_);
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
      EXPECT_TRUE(res.ok());
    }
    {
      auto res = ll->Commit(4);
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(3, res.value_);
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
      EXPECT_TRUE(res.ok());
    }
    {
      auto res = ll->Commit(2);
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(2, res.value_);
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
      EXPECT_TRUE(res.ok());
    }
    {
      auto res = ll->Commit(2);
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(2, res.value_);
    }
    {
      auto res = ll->GetCommitted();
      EXPECT_EQ(2, res.first);
      EXPECT_EQ(2, res.second);
    }
    {
      auto res = ll->Commit(1);
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(2, res.value_);
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
    EXPECT_TRUE(!res.ok());
  }
  Entry e;
  e.set_term(3);
  e.set_command("1");
  {
    auto res = log->Append(e);
    EXPECT_TRUE(res.ok());
  }
  {
    auto res = log->Get(1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(e.term(), res.value_->term());
    EXPECT_EQ(e.command(), res.value_->command());
  }
  {
    auto res = log->Get(2);
    EXPECT_TRUE(!res.ok());
  }
}

TEST(RaftTest, HasTest) {
  std::shared_ptr<KvStore> s = std::make_shared<KvStore>();
  auto log = buildLog(s);
  {
    auto res = log->Get(1);
    EXPECT_TRUE(!res.ok());
  }
  Entry e;
  e.set_term(2);
  e.set_command("1");
  {
    auto res = log->Append(e);
    EXPECT_TRUE(res.ok());
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
  auto r = buildLog();
  auto log = r.first;
  {
    Entry e;
    e.set_term(1);
    std::vector<std::string> cmds{"1", "2", "3"};
    for (const auto &c : cmds) {
      e.set_command(c);
      auto res = log->Append(e);
      EXPECT_TRUE(res.ok());
    }
  }
  {
    auto res = log->Range(0);
    int i = 1;
    for (const auto &c : *res) {
      EXPECT_EQ(1, c->term());
      EXPECT_EQ(std::to_string(i), c->command());
      i++;
    }
  }
}

TEST(RaftTest, LoadSaveTermTest) {
  auto r = buildLog();
  {
    auto log = buildLog(r.second);
    {
      auto res = log->LoadTerm();
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(0, std::get<0>(res.value_));
      EXPECT_EQ("", std::get<1>(res.value_));
    }
    std::string a("a");
    auto res = log->SaveTerm(1, a);
    EXPECT_TRUE(res.ok());
  }
  {
    auto log = buildLog(r.second);
    {
      auto res = log->LoadTerm();
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(1, std::get<0>(res.value_));
      EXPECT_EQ("a", std::get<1>(res.value_));
    }
    std::string a("c");
    auto res = log->SaveTerm(3, a);
    EXPECT_TRUE(res.ok());
  }
  {
    auto log = buildLog(r.second);
    {
      auto res = log->LoadTerm();
      EXPECT_TRUE(res.ok());
      EXPECT_EQ(3, std::get<0>(res.value_));
      EXPECT_EQ("c", std::get<1>(res.value_));
    }
    std::string a;
    auto res = log->SaveTerm(0, a);
    EXPECT_TRUE(res.ok());
  }
  {
    auto log = buildLog(r.second);
    auto res = log->LoadTerm();
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(0, std::get<0>(res.value_));
    EXPECT_EQ("", std::get<1>(res.value_));
  }
}

TEST(RaftTest, SpliceTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  {
    auto res = log->Append(e1);
    EXPECT_TRUE(res.ok());
  }
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  {
    auto res = log->Append(e2);
    EXPECT_TRUE(res.ok());
  }
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  {
    auto res = log->Append(e3);
    EXPECT_TRUE(res.ok());
  }
  Entry e4;
  e4.set_term(4);
  e4.set_command("4");

  {
    std::vector<Entry *> ts{&e3, &e4};
    auto res = log->Splice(2, 2, ts);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(4, res.value_);
  }
  std::vector<Entry *> es{&e1, &e2, &e3, &e4};
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(4, res.first);
    EXPECT_EQ(4, res.second);
  }
}

TEST(RaftTest, SpliceAllTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  std::vector<Entry *> es{&e1, &e2, &e3};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  Entry e4;
  e4.set_term(4);
  e4.set_command("10");
  Entry e4b;
  e4b.set_term(4);
  e4b.set_command("11");
  {
    std::vector<Entry *> ts{&e4, &e4b};
    auto res = log->Splice(0, 0, ts);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(2, res.value_);
  }

  es = {&e4, &e4b};
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(2, res.first);
    EXPECT_EQ(4, res.second);
  }
}

TEST(RaftTest, SpliceAppendTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");

  std::vector<Entry *> es{&e1, &e2};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  Entry e4;
  e4.set_term(4);
  e4.set_command("4");
  {
    es = {&e3, &e4};
    auto res = log->Splice(2, 2, es);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(4, res.value_);
  }

  es = {&e1, &e2, &e3, &e4};
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(4, res.first);
    EXPECT_EQ(4, res.second);
  }
}

TEST(RaftTest, SpliceBaseMissingTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");

  std::vector<Entry *> es{&e1, &e2};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  Entry e4;
  e4.set_term(4);
  e4.set_command("4");
  {
    std::vector<Entry *> ts{&e4};
    auto res = log->Splice(3, 3, ts);
    EXPECT_FALSE(res.ok());
  }

  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(2, res.first);
    EXPECT_EQ(2, res.second);
  }
}

TEST(RaftTest, SpliceBaseTermConflictTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");

  std::vector<Entry *> es{&e1, &e2};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  Entry e4;
  e4.set_term(4);
  e4.set_command("4");
  {
    std::vector<Entry *> ts{&e4};
    auto res = log->Splice(3, 3, ts);
    EXPECT_FALSE(res.ok());
  }
  {
    std::vector<Entry *> ts{&e4};
    auto res = log->Splice(2, 0, ts);
    EXPECT_FALSE(res.ok());
  }

  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(2, res.first);
    EXPECT_EQ(2, res.second);
  }
}

TEST(RaftTest, SpliceConflictTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  Entry e4;
  e4.set_term(4);
  e4.set_command("4");
  std::vector<Entry *> es{&e1, &e2, &e3, &e4};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }

  Entry e3b;
  e3b.set_term(3);
  e3b.set_command("b");
  Entry e3c;
  e3c.set_term(3);
  e3c.set_command("c");

  es = {&e3b, &e3c};
  {
    auto res = log->Splice(1, 1, es);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(3, res.value_);
  }
  es = {&e1, &e3b, &e3c};
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(3, res.first);
    EXPECT_EQ(3, res.second);
  }
}

TEST(RaftTest, SpliceOverlapTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  std::vector<Entry *> es{&e1, &e2, &e3};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }

  es = {&e2};
  {
    auto res = log->Splice(1, 1, es);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(3, res.value_);
  }
  es = {&e1, &e2, &e3};
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(3, res.first);
    EXPECT_EQ(3, res.second);
  }
}

TEST(RaftTest, TruncateTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  std::vector<Entry *> es{&e1, &e2, &e3};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  {
    auto res = log->Truncate(2);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(2, res.value_);
  }
  es = {&e1, &e2};
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->Get(3);
    EXPECT_FALSE(res.ok());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(2, res.first);
    EXPECT_EQ(2, res.second);
  }
}

TEST(RaftTest, TruncateBeyondTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  std::vector<Entry *> es{&e1, &e2, &e3};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  {
    auto res = log->Truncate(4);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(3, res.value_);
  }
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(es[i]->term(), res.value_->term());
    EXPECT_EQ(es[i]->command(), res.value_->command());
  }
  {
    auto res = log->Get(4);
    EXPECT_FALSE(res.ok());
  }
  {
    auto res = log->GetLast();
    EXPECT_EQ(3, res.first);
    EXPECT_EQ(3, res.second);
  }
}

TEST(RaftTest, TruncateCommittedTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  std::vector<Entry *> es{&e1, &e2, &e3};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  {
    auto res = log->Commit(2);
    EXPECT_TRUE(res.ok());
  }
  {
    auto res = log->Truncate(1);
    EXPECT_FALSE(res.ok());
  }
  {
    auto res = log->Truncate(2);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(2, res.value_);
  }
}

TEST(RaftTest, TruncateZeroTest) {
  auto r = buildLog();
  auto log = r.first;
  Entry e1;
  e1.set_term(1);
  e1.set_command("1");
  Entry e2;
  e2.set_term(2);
  e2.set_command("2");
  Entry e3;
  e3.set_term(3);
  e3.set_command("3");
  std::vector<Entry *> es{&e1, &e2, &e3};
  for (auto e : es) {
    auto res = log->Append(*e);
    EXPECT_TRUE(res.ok());
  }
  {
    auto res = log->Truncate(0);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(0, res.value_);
  }
  es = {&e1, &e2};
  for (int i = 0; i < es.size(); i++) {
    auto res = log->Get(i + 1);
    EXPECT_FALSE(res.ok());
  }
}

} // namespace toydb::raft

int main(int argc, char **argv) {
  InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
