ARG ROS2_DISTRO=jazzy
FROM ros:${ROS2_DISTRO}-ros-base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    less \
    libeigen3-dev \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-pip \
    python3-rosdep \
    ros-${ROS_DISTRO}-ament-cmake \
    ros-${ROS_DISTRO}-ament-cmake-auto \
    ros-${ROS_DISTRO}-ament-cmake-gtest \
    ros-${ROS_DISTRO}-ament-lint-auto \
    ros-${ROS_DISTRO}-ament-lint-common \
    ros-${ROS_DISTRO}-diagnostic-msgs \
    ros-${ROS_DISTRO}-geometry-msgs \
    ros-${ROS_DISTRO}-launch-testing \
    ros-${ROS_DISTRO}-launch-testing-ament-cmake \
    ros-${ROS_DISTRO}-launch-testing-ros \
    ros-${ROS_DISTRO}-nav-msgs \
    ros-${ROS_DISTRO}-rclcpp \
    ros-${ROS_DISTRO}-rclcpp-components \
    ros-${ROS_DISTRO}-rmw-cyclonedds-cpp \
    ros-${ROS_DISTRO}-rosidl-default-generators \
    ros-${ROS_DISTRO}-rosidl-default-runtime \
    ros-${ROS_DISTRO}-sensor-msgs \
    ros-${ROS_DISTRO}-std-msgs \
    ros-${ROS_DISTRO}-tf2 \
    ros-${ROS_DISTRO}-tf2-eigen \
    ros-${ROS_DISTRO}-tf2-geometry-msgs \
    ros-${ROS_DISTRO}-tf2-ros \
    ros-${ROS_DISTRO}-tf2-sensor-msgs \
    ros-${ROS_DISTRO}-visualization-msgs \
    sudo \
    vim \
 && rm -rf /var/lib/apt/lists/*

ARG USER_NAME=dev
ARG USER_UID=1000
ARG USER_GID=1000

COPY .docker/setup_user.sh /tmp/setup_user.sh
RUN /tmp/setup_user.sh && rm /tmp/setup_user.sh

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /home/${USER_NAME}/.bashrc \
 && echo '[ -f /opt/sq-kit/ros2_workspace/install/setup.bash ] && source /opt/sq-kit/ros2_workspace/install/setup.bash' >> /home/${USER_NAME}/.bashrc

RUN mkdir -p /opt/sq-kit/ros2_workspace/build /opt/sq-kit/ros2_workspace/install /opt/sq-kit/ros2_workspace/log \
 && chown -R ${USER_UID}:${USER_GID} /opt/sq-kit

USER ${USER_NAME}
WORKDIR /opt/sq-kit/ros2_workspace
ENV SHELL=/bin/bash

CMD ["sleep", "infinity"]
