#ifndef ROS1_COMPAT__RCLCPP__LOGGER_HPP_
#define ROS1_COMPAT__RCLCPP__LOGGER_HPP_

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

namespace rclcpp
{

class Logger
{
public:
  explicit Logger(const std::string & name)
  : spdlog_logger_(spdlog::default_logger()->clone(name))
  {
    spdlog_logger_->set_pattern("[%l] [%E.%f] [%n]: %v");
    spdlog_logger_->flush_on(spdlog::level::warn);
  }

  const char * get_name() const {return spdlog_logger_->name().c_str();}

  std::shared_ptr<spdlog::logger> get_spdlog_logger() const {return spdlog_logger_;}

private:
  std::shared_ptr<spdlog::logger> spdlog_logger_;
};

inline Logger get_logger(const std::string & name)
{
  return Logger(name);
}

}  // namespace rclcpp

#endif  // ROS1_COMPAT__RCLCPP__LOGGER_HPP_
