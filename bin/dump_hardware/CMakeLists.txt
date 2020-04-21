add_executable(dump_hardware
  dump_hardware.cpp
)

target_link_libraries(dump_hardware
  PUBLIC
    rvnsyncpoint
)

include(GNUInstallDirs)
install(TARGETS dump_hardware
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
