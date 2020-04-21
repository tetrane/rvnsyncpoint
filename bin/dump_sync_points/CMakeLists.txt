add_executable(dump_sync_points
  dump_sync_points.cpp
)

target_link_libraries(dump_sync_points
  PUBLIC
    rvnsyncpoint
)

include(GNUInstallDirs)
install(TARGETS dump_sync_points
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
