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

#ifndef RCLCPP__LOGGER_HPP_
#define RCLCPP__LOGGER_HPP_

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace rclcpp
{

class Logger
{
public:
  explicit Logger(const std::string & name) : spdlog_logger_(spdlog::default_logger()->clone(name))
  {
    spdlog_logger_->set_pattern("[%l] [%E.%f] [%n]: %v");
    spdlog_logger_->flush_on(spdlog::level::warn);
  }

  const char * get_name() const { return spdlog_logger_->name().c_str(); }

  std::shared_ptr<spdlog::logger> get_spdlog_logger() const { return spdlog_logger_; }

private:
  std::shared_ptr<spdlog::logger> spdlog_logger_;
};

inline Logger get_logger(const std::string & name) { return Logger(name); }

}  // namespace rclcpp

#endif  // RCLCPP__LOGGER_HPP_
