// Copyright 2026 SEQSENSE, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SQ_ROS1_COMPAT__MSG_PTR_HPP_
#define SQ_ROS1_COMPAT__MSG_PTR_HPP_

// boost ↔ std shared_ptr adapters using the aliasing constructor to preserve
// source lifetime without copying the message.

#include <boost/shared_ptr.hpp>
#include <memory>  // NOLINT(build/include_order)

namespace sq_ros1_compat
{

template <typename T>
inline std::shared_ptr<const T> to_std(const boost::shared_ptr<const T> & b)
{
  return std::shared_ptr<const T>(b.get(), [b](const T *) {});
}

template <typename T>
inline std::shared_ptr<T> to_std(const boost::shared_ptr<T> & b)
{
  return std::shared_ptr<T>(b.get(), [b](T *) {});
}

template <typename T>
inline boost::shared_ptr<T> to_boost(const std::shared_ptr<T> & s)
{
  return boost::shared_ptr<T>(s, s.get());
}

template <typename T>
inline boost::shared_ptr<const T> to_boost(const std::shared_ptr<const T> & s)
{
  return boost::shared_ptr<const T>(s, s.get());
}

}  // namespace sq_ros1_compat

#endif  // SQ_ROS1_COMPAT__MSG_PTR_HPP_
