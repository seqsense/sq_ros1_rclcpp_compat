ARG ROS2_DISTRO=jazzy
FROM ros:${ROS2_DISTRO}-ros-base

ARG USER_NAME=dev
ARG USER_UID=1000
ARG USER_GID=1000

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    sudo \
    vim \
    less \
    python3-pip \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    ros-${ROS_DISTRO}-ament-cmake \
    ros-${ROS_DISTRO}-ament-cmake-auto \
    ros-${ROS_DISTRO}-ament-cmake-gtest \
    ros-${ROS_DISTRO}-ament-lint-auto \
    ros-${ROS_DISTRO}-ament-lint-common \
    ros-${ROS_DISTRO}-launch-testing \
    ros-${ROS_DISTRO}-launch-testing-ament-cmake \
    ros-${ROS_DISTRO}-launch-testing-ros \
    ros-${ROS_DISTRO}-rclcpp \
    ros-${ROS_DISTRO}-rclcpp-components \
    ros-${ROS_DISTRO}-rmw-cyclonedds-cpp \
    ros-${ROS_DISTRO}-std-msgs \
    ros-${ROS_DISTRO}-sensor-msgs \
    ros-${ROS_DISTRO}-geometry-msgs \
    ros-${ROS_DISTRO}-nav-msgs \
    ros-${ROS_DISTRO}-visualization-msgs \
    ros-${ROS_DISTRO}-diagnostic-msgs \
    ros-${ROS_DISTRO}-tf2 \
    ros-${ROS_DISTRO}-tf2-ros \
    ros-${ROS_DISTRO}-tf2-geometry-msgs \
    ros-${ROS_DISTRO}-tf2-eigen \
    ros-${ROS_DISTRO}-tf2-sensor-msgs \
    ros-${ROS_DISTRO}-rosidl-default-generators \
    ros-${ROS_DISTRO}-rosidl-default-runtime \
    libeigen3-dev \
 && rm -rf /var/lib/apt/lists/*

RUN if getent passwd ${USER_UID} > /dev/null; then \
      EXISTING_USER=$(getent passwd ${USER_UID} | cut -d: -f1); \
      userdel -r "$EXISTING_USER" 2>/dev/null || true; \
    fi \
 && if getent group ${USER_GID} > /dev/null; then \
      EXISTING_GROUP=$(getent group ${USER_GID} | cut -d: -f1); \
      if [ "$EXISTING_GROUP" != "${USER_NAME}" ]; then groupdel "$EXISTING_GROUP" 2>/dev/null || true; fi; \
    fi \
 && if ! getent group ${USER_GID} > /dev/null; then groupadd --gid ${USER_GID} ${USER_NAME}; fi \
 && useradd --uid ${USER_UID} --gid ${USER_GID} -m -s /bin/bash ${USER_NAME} \
 && echo "${USER_NAME} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USER_NAME} \
 && chmod 0440 /etc/sudoers.d/${USER_NAME}

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /home/${USER_NAME}/.bashrc \
 && echo '[ -f /opt/sq-kit/ros2_workspace/install/setup.bash ] && source /opt/sq-kit/ros2_workspace/install/setup.bash' >> /home/${USER_NAME}/.bashrc

RUN mkdir -p /opt/sq-kit/ros2_workspace/build /opt/sq-kit/ros2_workspace/install /opt/sq-kit/ros2_workspace/log \
 && chown -R ${USER_UID}:${USER_GID} /opt/sq-kit

USER ${USER_NAME}
WORKDIR /opt/sq-kit/ros2_workspace
ENV SHELL=/bin/bash

CMD ["bash", "-c", "tail -f /dev/null"]
