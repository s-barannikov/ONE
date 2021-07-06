/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "passes/optimizations/SinkTranspose.h"
#include "passes/optimizations/OptimizationUtils.h"
#include "mir/ops/AddOp.h"
#include "mir/ops/ConstantOp.h"
#include "mir/ops/TransposeOp.h"
#include "mir/ops/ConcatOp.h"
#include "mir/ops/ReluOp.h"
#include "mir/Graph.h"
#include "mir/GraphPatternMatcher.h"

#include "MirInterpreter.h"

#include <string>
#include <algorithm>

namespace nnc
{

using namespace mir;
using namespace opt_util;

static std::vector<std::size_t> getReversedPermutation(const std::vector<std::size_t> &perm)
{
  std::vector<std::size_t> result(perm.size());
  for (std::size_t i = 0; i < perm.size(); ++i)
  {
    result.at(perm[i]) = i;
  }
  return result;
}

PassData SinkTranspose::run(PassData data)
{
  auto g = static_cast<Graph *>(data);

  for (auto *op : getSortedNodes(g))
  {
    if (auto *add_op = dynamic_cast<ops::AddOp *>(op))
    {
      auto *input_1 = dynamic_cast<ops::TransposeOp *>(add_op->getInput(0)->getNode());
      auto *input_2 = dynamic_cast<ops::ConstantOp *>(add_op->getInput(1)->getNode());
      if (input_1 != nullptr && input_2 != nullptr)
      {
        auto *new_transpose = g->create<ops::TransposeOp>(
            input_2->getOutput(0), getReversedPermutation(input_1->getAxisOrder()));
        new_transpose = foldConstants(g, new_transpose);
        auto *new_add = g->create<ops::AddOp>(input_1->getInput(0), new_transpose->getOutput(0));
        auto *result = g->create<ops::TransposeOp>(new_add->getOutput(0), input_1->getAxisOrder());

        g->replaceNode(op, result);
      }
      else
      {
        auto *input_3 = dynamic_cast<ops::TransposeOp *>(add_op->getInput(1)->getNode());
        if (input_1 && input_3 && input_1->getAxisOrder() == input_3->getAxisOrder())
        {
          auto *new_add = g->create<ops::AddOp>(input_1->getInput(0), input_3->getInput(0));
          auto *new_transpose =
              g->create<ops::TransposeOp>(new_add->getOutput(0), input_1->getAxisOrder());

          g->replaceNode(op, new_transpose);
        }
      }
    }
  }

  assert(g); // NOLINT
  GraphPatternMatcher matcher(g);
  auto is_tr = [](const Operation *op1) { return op1->getType() == Operation::Type::transpose; };
  auto is_relu = [](const Operation *op2) { return op2->getType() == Operation::Type::ReLU; };
  auto is_concat = [](const Operation *op2) { return op2->getType() == Operation::Type::concat; };
  std::vector<std::pair<Operation *, Operation *>> matches;

  // sink transpose below ReLU
  matches = matcher.matchEdge(is_tr, is_relu);
  for (auto pair : matches)
  {
    swapAdjacent(g, pair.first, pair.second);
  }

  // sink transpose through Concat
  auto v_matches = matcher.matchUpBush(is_tr, is_concat);
  for (const auto &pair : v_matches)
  {
    std::vector<Operation *> trs = pair.first;
    auto *concat = dynamic_cast<ops::ConcatOp *>(pair.second);
    auto axis_order = dynamic_cast<ops::TransposeOp *>(trs[0])->getAxisOrder();
    if (std::all_of(trs.begin(), trs.end(), [&axis_order](Operation *tr) {
          return dynamic_cast<ops::TransposeOp *>(tr)->getAxisOrder() == axis_order;
        }))
    {
      std::vector<Operation::Output *> prev_trans;
      prev_trans.reserve(trs.size());
      for (auto transpose : trs)
      {
        prev_trans.emplace_back(transpose->getInput(0));
      }
      auto new_concat = g->create<ops::ConcatOp>(prev_trans, axis_order[concat->getAxis()]);
      auto new_transpose = g->create<ops::TransposeOp>(new_concat->getOutput(0), axis_order);
      // removes old concat
      g->replaceNode(concat, new_transpose);
      for (auto tr : trs)
      {
        removeNodeIfUnused(g, tr);
      }
    }
  }

  return g;
}

} // namespace nnc
