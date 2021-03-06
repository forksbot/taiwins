# Try to find Wayland on a Unix system
#
# This will define:
#
#   WAYLAND_FOUND       - True if Wayland is found
#   WAYLAND_LIBRARIES   - Link these to use Wayland
#   WAYLAND_INCLUDE_DIR - Include directory for Wayland
#   WAYLAND_DEFINITIONS - Compiler flags for using Wayland
#
# In addition the following more fine grained variables will be defined:
#
#   WAYLAND_CLIENT_FOUND  WAYLAND_CLIENT_INCLUDE_DIR  WAYLAND_CLIENT_LIBRARIES
#   WAYLAND_SERVER_FOUND  WAYLAND_SERVER_INCLUDE_DIR  WAYLAND_SERVER_LIBRARIES
#   WAYLAND_EGL_FOUND     WAYLAND_EGL_INCLUDE_DIR     WAYLAND_EGL_LIBRARIES
#   WAYLAND_CURSOR_FOUND  WAYLAND_CURSOR_INCLUDE_DIR  WAYLAND_CURSOR_LIBRARIES
#
# Copyright (c) 2013 Martin Gräßlin <mgraesslin@kde.org>
# Copyright (c) 2019 Xichen Zhou
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
# it cannot be win32...

IF (NOT WIN32)
  IF (WAYLAND_INCLUDE_DIR AND WAYLAND_LIBRARIES)
    # In the cache already
    SET(WAYLAND_FIND_QUIETLY TRUE)
  ENDIF ()

  # Use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  FIND_PACKAGE(PkgConfig)
  PKG_CHECK_MODULES(PKG_WAYLAND QUIET wayland-client wayland-server wayland-egl wayland-cursor)

  SET(WAYLAND_DEFINITIONS ${PKG_WAYLAND_CFLAGS})

  FIND_PATH(WAYLAND_CLIENT_INCLUDE_DIR  NAMES wayland-client.h HINTS ${PKG_WAYLAND_INCLUDE_DIRS})
  FIND_PATH(WAYLAND_SERVER_INCLUDE_DIR  NAMES wayland-server.h HINTS ${PKG_WAYLAND_INCLUDE_DIRS})
  FIND_PATH(WAYLAND_EGL_INCLUDE_DIR     NAMES wayland-egl.h    HINTS ${PKG_WAYLAND_INCLUDE_DIRS})
  FIND_PATH(WAYLAND_CURSOR_INCLUDE_DIR  NAMES wayland-cursor.h HINTS ${PKG_WAYLAND_INCLUDE_DIRS})

  FIND_LIBRARY(WAYLAND_CLIENT_LIBRARIES NAMES wayland-client   HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
  FIND_LIBRARY(WAYLAND_SERVER_LIBRARIES NAMES wayland-server   HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
  FIND_LIBRARY(WAYLAND_EGL_LIBRARIES    NAMES wayland-egl      HINTS ${PKG_WAYLAND_LIBRARY_DIRS})
  FIND_LIBRARY(WAYLAND_CURSOR_LIBRARIES NAMES wayland-cursor   HINTS ${PKG_WAYLAND_LIBRARY_DIRS})

  set(WAYLAND_INCLUDE_DIR ${WAYLAND_CLIENT_INCLUDE_DIR} ${WAYLAND_SERVER_INCLUDE_DIR} ${WAYLAND_EGL_INCLUDE_DIR} ${WAYLAND_CURSOR_INCLUDE_DIR})

  set(WAYLAND_LIBRARIES ${WAYLAND_CLIENT_LIBRARIES} ${WAYLAND_SERVER_LIBRARIES} ${WAYLAND_EGL_LIBRARIES} ${WAYLAND_CURSOR_LIBRARIES})

  list(REMOVE_DUPLICATES WAYLAND_INCLUDE_DIR)

  include(FindPackageHandleStandardArgs)

  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Wayland DEFAULT_MSG
    WAYLAND_CLIENT_LIBRARIES  WAYLAND_CLIENT_INCLUDE_DIR
    WAYLAND_SERVER_LIBRARIES  WAYLAND_SERVER_INCLUDE_DIR
    WAYLAND_EGL_LIBRARIES     WAYLAND_EGL_INCLUDE_DIR
    WAYLAND_CURSOR_LIBRARIES  WAYLAND_CURSOR_INCLUDE_DIR
    WAYLAND_LIBRARIES         WAYLAND_INCLUDE_DIR
    )

  MARK_AS_ADVANCED(
    WAYLAND_INCLUDE_DIR         WAYLAND_LIBRARIES
    WAYLAND_CLIENT_INCLUDE_DIR  WAYLAND_CLIENT_LIBRARIES
    WAYLAND_SERVER_INCLUDE_DIR  WAYLAND_SERVER_LIBRARIES
    WAYLAND_EGL_INCLUDE_DIR     WAYLAND_EGL_LIBRARIES
    WAYLAND_CURSOR_INCLUDE_DIR  WAYLAND_CURSOR_LIBRARIES
    )

  if (Wayland_FOUND AND NOT TARGET Wayland::Client)
    add_library(Wayland::Client UNKNOWN IMPORTED)
    set_target_properties(Wayland::Client PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${WAYLAND_CLIENT_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_CLIENT_INCLUDE_DIR}"
      )
  endif()

  if (Wayland_FOUND AND NOT TARGET Wayland::Server)
    add_library(Wayland::Server UNKNOWN IMPORTED)
    set_target_properties(Wayland::Server PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${WAYLAND_SERVER_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_SERVER_INCLUDE_DIR}"
      )
  endif()

  if (Wayland_FOUND AND NOT TARGET Wayland::EGL)
    add_library(Wayland::EGL UNKNOWN IMPORTED)
    set_target_properties(Wayland::EGL PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${WAYLAND_EGL_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_EGL_INCLUDE_DIR}"
      )
  endif()

  if (Wayland_FOUND AND NOT TARGET Wayland::Cursor)
    add_library(Wayland::Cursor UNKNOWN IMPORTED)
    set_target_properties(Wayland::Cursor PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION "${WAYLAND_CURSOR_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${WAYLAND_CURSOR_INCLUDE_DIR}"
      )
  endif()

ENDIF ()
