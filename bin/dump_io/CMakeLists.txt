add_executable(dump_io
  dump_io.cpp
)

target_link_libraries(dump_io
  PUBLIC
    rvnsyncpoint
)

include(GNUInstallDirs)
install(TARGETS dump_io
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
