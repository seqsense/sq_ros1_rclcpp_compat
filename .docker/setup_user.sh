#!/bin/bash
set -e
if getent passwd "${USER_UID}" > /dev/null; then
    EXISTING_USER=$(getent passwd "${USER_UID}" | cut -d: -f1)
    userdel -r "$EXISTING_USER" 2>/dev/null || true
fi
if getent group "${USER_GID}" > /dev/null; then
    EXISTING_GROUP=$(getent group "${USER_GID}" | cut -d: -f1)
    if [ "$EXISTING_GROUP" != "${USER_NAME}" ]; then groupdel "$EXISTING_GROUP" 2>/dev/null || true; fi
fi
if ! getent group "${USER_GID}" > /dev/null; then groupadd --gid "${USER_GID}" "${USER_NAME}"; fi
useradd --uid "${USER_UID}" --gid "${USER_GID}" -m -s /bin/bash "${USER_NAME}"
echo "${USER_NAME} ALL=(ALL) NOPASSWD:ALL" > "/etc/sudoers.d/${USER_NAME}"
chmod 0440 "/etc/sudoers.d/${USER_NAME}"
