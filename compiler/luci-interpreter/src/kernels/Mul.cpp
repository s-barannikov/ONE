/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
 * Copyright 2019 The TensorFlow Authors. All Rights Reserved.
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

#include "kernels/Mul.h"

#include "kernels/Utils.h"

#include <tensorflow/lite/kernels/internal/optimized/optimized_ops.h>
#include <tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h>

#include <stdexcept>

namespace luci_interpreter
{
namespace kernels
{

Mul::Mul(const Tensor *input1, const Tensor *input2, Tensor *output, const MulParams &params)
    : KernelWithParams<MulParams>({input1, input2}, {output}, params)
{
}

void Mul::configure()
{
  assert(input1()->element_type() == input2()->element_type());
  output()->resize(calculateShapeForBroadcast(input1()->shape(), input2()->shape()));
}

void Mul::execute() const
{
  switch (input1()->element_type())
  {
    case DataType::FLOAT32:
      evalFloat();
      break;
    default:
      throw std::runtime_error("Unsupported type.");
  }
}

void Mul::evalFloat() const
{
  float activation_min{};
  float activation_max{};
  calculateActivationRange(_params.activation, &activation_min, &activation_max);

  tflite::ArithmeticParams params{};
  params.float_activation_min = activation_min;
  params.float_activation_max = activation_max;

  const bool need_broadcast = tflite::reference_ops::ProcessBroadcastShapes(
      getTensorShape(input1()), getTensorShape(input2()), &params);

  if (need_broadcast)
  {
    tflite::optimized_ops::BroadcastMul4DSlow(
        params, getTensorShape(input1()), getTensorData<float>(input1()), getTensorShape(input2()),
        getTensorData<float>(input2()), getTensorShape(output()), getTensorData<float>(output()));
  }
  else
  {
    tflite::optimized_ops::Mul(params, getTensorShape(input1()), getTensorData<float>(input1()),
                               getTensorShape(input2()), getTensorData<float>(input2()),
                               getTensorShape(output()), getTensorData<float>(output()));
  }
}

} // namespace kernels
} // namespace luci_interpreter
