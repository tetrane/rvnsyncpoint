add_executable(reorder_hardware
  reorder_hardware.cpp
)

target_link_libraries(reorder_hardware
  PUBLIC
    rvnsyncpoint
)

include(GNUInstallDirs)
install(TARGETS reorder_hardware
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
