#ifndef ROS1_COMPAT__RCLCPP__NODE_HPP_
#define ROS1_COMPAT__RCLCPP__NODE_HPP_

// ROS 1 compat: Minimal rclcpp::Node for parameter access.
// Allows shared logic to use declare_parameter / get_parameter with ROS2 syntax.
// On ROS1, delegates to ros::NodeHandle::param().

#include <ros/ros.h>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "rclcpp/logger.hpp"

namespace rclcpp
{

/// Minimal parameter value holder (subset of rclcpp::ParameterValue)
class ParameterValue
{
public:
  ParameterValue() : type_(TYPE_NOT_SET) {}
  explicit ParameterValue(bool v) : type_(TYPE_BOOL), bool_val_(v) {}
  explicit ParameterValue(int v) : type_(TYPE_INT), int_val_(v) {}
  explicit ParameterValue(int64_t v) : type_(TYPE_INT), int_val_(v) {}
  explicit ParameterValue(double v) : type_(TYPE_DOUBLE), double_val_(v) {}
  explicit ParameterValue(const std::string & v) : type_(TYPE_STRING), string_val_(v) {}
  explicit ParameterValue(const char * v) : type_(TYPE_STRING), string_val_(v) {}
  explicit ParameterValue(const std::vector<double> & v)
  : type_(TYPE_DOUBLE_ARRAY), double_array_val_(v) {}
  explicit ParameterValue(const std::vector<int64_t> & v)
  : type_(TYPE_INT_ARRAY), int_array_val_(v) {}
  explicit ParameterValue(const std::vector<std::string> & v)
  : type_(TYPE_STRING_ARRAY), string_array_val_(v) {}

  template<typename T>
  T get() const;

private:
  enum Type
  {
    TYPE_NOT_SET,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_DOUBLE_ARRAY,
    TYPE_INT_ARRAY,
    TYPE_STRING_ARRAY
  };
  Type type_;
  bool bool_val_ = false;
  int64_t int_val_ = 0;
  double double_val_ = 0.0;
  std::string string_val_;
  std::vector<double> double_array_val_;
  std::vector<int64_t> int_array_val_;
  std::vector<std::string> string_array_val_;
};

template<>
inline bool ParameterValue::get<bool>() const { return bool_val_; }
template<>
inline int ParameterValue::get<int>() const { return static_cast<int>(int_val_); }
template<>
inline int64_t ParameterValue::get<int64_t>() const { return int_val_; }
template<>
inline double ParameterValue::get<double>() const { return double_val_; }
template<>
inline std::string ParameterValue::get<std::string>() const { return string_val_; }
template<>
inline std::vector<double> ParameterValue::get<std::vector<double>>() const
{
  return double_array_val_;
}
template<>
inline std::vector<int64_t> ParameterValue::get<std::vector<int64_t>>() const
{
  return int_array_val_;
}
template<>
inline std::vector<std::string> ParameterValue::get<std::vector<std::string>>() const
{
  return string_array_val_;
}

/// Minimal rclcpp::Parameter
class Parameter
{
public:
  Parameter() = default;
  Parameter(const std::string & name, const ParameterValue & value)
  : name_(name), value_(value) {}

  const std::string & get_name() const { return name_; }
  const ParameterValue & get_value() const { return value_; }

  template<typename T>
  T as() const { return value_.get<T>(); }

  bool as_bool() const { return value_.get<bool>(); }
  int64_t as_int() const { return value_.get<int64_t>(); }
  double as_double() const { return value_.get<double>(); }
  std::string as_string() const { return value_.get<std::string>(); }
  std::vector<double> as_double_array() const { return value_.get<std::vector<double>>(); }

private:
  std::string name_;
  ParameterValue value_;
};

/// Node options (minimal, matches rclcpp::NodeOptions constructor arg)
struct NodeOptions
{
};

/// Minimal rclcpp::Node backed by ros::NodeHandle on ROS1.
/// Provides declare_parameter / get_parameter for shared logic.
class Node
{
public:
  explicit Node(const std::string & node_name, const NodeOptions & = NodeOptions())
  : name_(node_name)
  , nh_("~")
  , logger_(rclcpp::get_logger(node_name))
  {
  }

  Logger get_logger() const { return logger_; }

  /// Declare a parameter with default value. Reads from ROS1 param server.
  template<typename T>
  T declare_parameter(const std::string & name, const T & default_value)
  {
    const std::string ros1_name = dotToSlash(name);
    T value;
    nh_.param(ros1_name, value, default_value);
    params_[name] = Parameter(name, ParameterValue(value));
    return value;
  }

  /// Declare a parameter without default (must exist on param server).
  template<typename T>
  T declare_parameter(const std::string & name)
  {
    const std::string ros1_name = dotToSlash(name);
    T value;
    if (!nh_.getParam(ros1_name, value))
    {
      throw std::runtime_error("Parameter '" + name + "' not declared and has no default");
    }
    params_[name] = Parameter(name, ParameterValue(value));
    return value;
  }

  /// Get a previously declared parameter.
  bool get_parameter(const std::string & name, Parameter & param) const
  {
    auto it = params_.find(name);
    if (it != params_.end())
    {
      param = it->second;
      return true;
    }
    return false;
  }

  /// Get parameter value directly into typed variable.
  template<typename T>
  bool get_parameter(const std::string & name, T & value) const
  {
    auto it = params_.find(name);
    if (it != params_.end())
    {
      value = it->second.as<T>();
      return true;
    }
    return false;
  }

  /// Check if parameter has been declared.
  bool has_parameter(const std::string & name) const
  {
    return params_.count(name) > 0 || nh_.hasParam(dotToSlash(name));
  }

  /// Get current time (matches rclcpp::Node::now())
  rclcpp::Time now() const
  {
    return rclcpp::Time(ros::Time::now());
  }

  const std::string & get_name() const { return name_; }

private:
  /// Convert ROS2-style dot-separated parameter names to ROS1-style slash-separated.
  static std::string dotToSlash(const std::string & name)
  {
    std::string result = name;
    for (auto & ch : result)
    {
      if (ch == '.')
      {
        ch = '/';
      }
    }
    return result;
  }

  std::string name_;
  ros::NodeHandle nh_;
  Logger logger_;
  std::map<std::string, Parameter> params_;
};

}  // namespace rclcpp

#endif  // ROS1_COMPAT__RCLCPP__NODE_HPP_
