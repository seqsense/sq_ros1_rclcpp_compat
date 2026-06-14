ARG ROS1_DISTRO=noetic

# ─── Stage: Noetic on Ubuntu 20.04 (official OSRF image) ────────────────────
FROM ros:noetic-ros-base AS stage_noetic

# ─── Stage: ROS-O (ros-one) on Ubuntu 24.04 ─────────────────────────────────
# Repository: https://ros.packages.techfak.net  (github.com/ros-o/ros-o)
FROM ubuntu:noble AS stage_one
ENV ROS_DISTRO=one \
    DEBIAN_FRONTEND=noninteractive
# Step 1: register the ros-one apt repository.
# python3-rosdep is NOT in Ubuntu noble standard repos; it comes from ros-one,
# so the repository must be added before any ROS packages are installed.
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
 && mkdir -p /etc/apt/keyrings \
 && curl -sSL https://ros.packages.techfak.net/gpg.key \
         -o /etc/apt/keyrings/ros-one-keyring.gpg \
 && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/ros-one-keyring.gpg] \
     https://ros.packages.techfak.net noble main" \
     | tee /etc/apt/sources.list.d/ros1.list \
 && rm -rf /var/lib/apt/lists/*
# Step 2: install ROS-O base and rosdep (now that the repo is registered).
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-one-ros-base \
    python3-rosdep \
 && rm -rf /var/lib/apt/lists/*
# Pre-register the ros-one rosdep source so `rosdep update` sees ros-one packages.
RUN rosdep init \
 && echo "yaml https://ros.packages.techfak.net/ros-one.yaml one" \
     > /etc/ros/rosdep/sources.list.d/1-ros-one.list

# ─── Active base: selected by ROS1_DISTRO build arg ──────────────────────────
FROM stage_${ROS1_DISTRO}

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
    python3-catkin-tools \
    python3-rosdep \
    python3-osrf-pycommon \
    ros-${ROS_DISTRO}-rostest \
    ros-${ROS_DISTRO}-roslint \
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
    ros-${ROS_DISTRO}-message-generation \
    ros-${ROS_DISTRO}-message-runtime \
    libspdlog-dev \
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
 && echo '[ -f /opt/sq-kit/workspace/devel/setup.bash ] && source /opt/sq-kit/workspace/devel/setup.bash' >> /home/${USER_NAME}/.bashrc

RUN mkdir -p /opt/sq-kit/workspace/build /opt/sq-kit/workspace/devel /opt/sq-kit/workspace/logs \
 && chown -R ${USER_UID}:${USER_GID} /opt/sq-kit

USER ${USER_NAME}
WORKDIR /opt/sq-kit/workspace
ENV SHELL=/bin/bash

CMD ["bash", "-c", "tail -f /dev/null"]
