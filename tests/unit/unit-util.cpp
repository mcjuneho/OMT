/*********************                                           */
/*! \file unit-util.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Unit tests for util functions
**
**
**/

#include <utility>
#include <vector>

#include "utils.h"

#include "available_solvers.h"
#include "gtest/gtest.h"
#include "smt.h"

using namespace smt;
using namespace std;

namespace smt_tests {

class UnitUtilTests : public ::testing::Test,
                      public ::testing::WithParamInterface<SolverEnum>
{
 protected:
  void SetUp() override
  {
    s = create_solver(GetParam());

    boolsort = s->make_sort(BOOL);
    for (size_t i = 0; i < 30; ++i)
    {
      symbols.push_back(s->make_symbol("x" + std::to_string(i), boolsort));
    }
  }
  SmtSolver s;
  Sort boolsort;
  TermVec symbols;
};

TEST_P(UnitUtilTests, ConjunctivePartition)
{
  Term conjunction = symbols[0];
  for (size_t j = 1; j < symbols.size(); ++j)
  {
    conjunction = s->make_term(And, conjunction, symbols[j]);
  }

  TermVec conjuncts;
  // boolean argument means to include BVAnd
  // if over 1-bit variables
  // then this will work for Boolector even without logging
  conjunctive_partition(conjunction, conjuncts, true);
  ASSERT_EQ(symbols.size(), conjuncts.size());

  // order not necessarily maintained
  UnorderedTermSet conjuncts_set(conjuncts.begin(), conjuncts.end());

  for (size_t j = 0; j < symbols.size(); ++j)
  {
    ASSERT_TRUE(conjuncts_set.find(symbols[j]) != conjuncts_set.end());
  }
}

TEST_P(UnitUtilTests, DisjunctivePartition)
{
  if (s->get_solver_enum() == BTOR)
  {
    // Boolector rewrites Ors as Not And
    // it's equivalent, but disjunctive partition won't work
    return;
  }

  Term disjunction = symbols[0];
  for (size_t j = 1; j < symbols.size(); ++j)
  {
    disjunction = s->make_term(Or, disjunction, symbols[j]);
  }

  TermVec disjuncts;
  // boolean argument means to include BVOr
  // if over 1-bit variables
  // then this will work for Boolector even without logging
  disjunctive_partition(disjunction, disjuncts, true);
  ASSERT_EQ(symbols.size(), disjuncts.size());

  // order not necessarily maintained
  UnorderedTermSet disjuncts_set(disjuncts.begin(), disjuncts.end());

  for (size_t j = 0; j < symbols.size(); ++j)
  {
    ASSERT_TRUE(disjuncts_set.find(symbols[j]) != disjuncts_set.end());
  }
}

INSTANTIATE_TEST_SUITE_P(ParameterizedUnitUtilTests,
                         UnitUtilTests,
                         testing::ValuesIn(filter_solver_enums({ TERMITER })));

}  // namespace smt_tests
