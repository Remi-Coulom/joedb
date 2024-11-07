include_guard(GLOBAL)

#############################################################################
if(WIN32)
#############################################################################
 message("== win32")
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 add_definitions(-DWIN32_LEAN_AND_MEAN)
 add_definitions(-DNOMINMAX)
endif()

#############################################################################
if(UNIX)
#############################################################################
 message("== unix")
 add_definitions(-D_FILE_OFFSET_BITS=64)
 if (CMAKE_CROSSCOMPILING_EMULATOR)
  # This is necessary for qemu to work, although OFD macros are defined
  add_definitions(-DJOEDB_HAS_BROKEN_POSIX_LOCKING)
 endif()
endif()
