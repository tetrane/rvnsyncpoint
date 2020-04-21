add_executable(dump_sync_points_data
  dump_sync_points_data.cpp
)

target_link_libraries(dump_sync_points_data
  PUBLIC
    rvnsyncpoint
)

include(GNUInstallDirs)
install(TARGETS dump_sync_points_data
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
