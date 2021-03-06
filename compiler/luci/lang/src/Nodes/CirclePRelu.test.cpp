/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
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

#include "luci/IR/Nodes/CirclePRelu.h"

#include "luci/IR/CircleDialect.h"
#include "luci/IR/CircleNodeVisitor.h"

#include <gtest/gtest.h>

TEST(CirclePReluTest, constructor_P)
{
  luci::CirclePRelu prelu_node;

  ASSERT_EQ(luci::CircleDialect::get(), prelu_node.dialect());
  ASSERT_EQ(luci::CircleOpcode::PRELU, prelu_node.opcode());

  ASSERT_EQ(nullptr, prelu_node.input());
  ASSERT_EQ(nullptr, prelu_node.alpha());
}

TEST(CirclePReluTest, input_NEG)
{
  luci::CirclePRelu prelu_node;
  luci::CirclePRelu node;

  prelu_node.input(&node);
  prelu_node.alpha(&node);
  ASSERT_NE(nullptr, prelu_node.input());
  ASSERT_NE(nullptr, prelu_node.alpha());

  prelu_node.input(nullptr);
  prelu_node.alpha(nullptr);
  ASSERT_EQ(nullptr, prelu_node.input());
  ASSERT_EQ(nullptr, prelu_node.alpha());
}

TEST(CirclePReluTest, arity_NEG)
{
  luci::CirclePRelu prelu_node;

  ASSERT_NO_THROW(prelu_node.arg(1));
  ASSERT_THROW(prelu_node.arg(2), std::out_of_range);
}

TEST(CirclePReluTest, visit_mutable_NEG)
{
  struct TestVisitor final : public luci::CircleNodeMutableVisitor<void>
  {
  };

  luci::CirclePRelu prelu_node;

  TestVisitor tv;
  ASSERT_THROW(prelu_node.accept(&tv), std::exception);
}

TEST(CirclePReluTest, visit_NEG)
{
  struct TestVisitor final : public luci::CircleNodeVisitor<void>
  {
  };

  luci::CirclePRelu prelu_node;

  TestVisitor tv;
  ASSERT_THROW(prelu_node.accept(&tv), std::exception);
}
