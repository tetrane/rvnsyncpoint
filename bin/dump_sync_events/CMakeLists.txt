add_executable(dump_sync_events
  dump_sync_events.cpp
)

target_link_libraries(dump_sync_events
  PUBLIC
    rvnsyncpoint
)

include(GNUInstallDirs)
install(TARGETS dump_sync_events
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
