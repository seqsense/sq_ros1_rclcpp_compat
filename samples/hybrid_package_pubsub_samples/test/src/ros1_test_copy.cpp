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

#include <gtest/gtest.h>

#include "hybrid_package_msgs/StampedPointer.h"
#include "ros/ros.h"

class CopyTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    nh_ = std::make_unique<ros::NodeHandle>();
    copy_confirmed_ = false;
    sub_ = nh_->subscribe("stamped_pointer", 10, &CopyTest::callback, this);
  }

  void callback(const hybrid_package_msgs::StampedPointer::ConstPtr & msg)
  {
    const uint64_t received_ptr = reinterpret_cast<uint64_t>(msg.get());
    if (msg->pointer_value != received_ptr) {
      copy_confirmed_ = true;
    }
  }

  std::unique_ptr<ros::NodeHandle> nh_;
  ros::Subscriber sub_;
  bool copy_confirmed_;
};

TEST_F(CopyTest, MessageIsCopied)
{
  const ros::Time deadline = ros::Time::now() + ros::Duration(20.0);
  ros::Rate rate(10.0);
  while (ros::ok() && !copy_confirmed_ && ros::Time::now() < deadline) {
    ros::spinOnce();
    rate.sleep();
  }
  EXPECT_TRUE(copy_confirmed_) << "Message should be copied in inter-process communication";
}

int main(int argc, char ** argv)
{
  ros::init(argc, argv, "test_copy");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
