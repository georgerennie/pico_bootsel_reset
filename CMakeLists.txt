add_library(bootsel_reset INTERFACE)

if (PICO_ON_DEVICE)
	target_sources(bootsel_reset INTERFACE
		${CMAKE_CURRENT_LIST_DIR}/bootsel_reset.c
	)

	target_link_libraries(bootsel_reset INTERFACE
		pico_bootrom
		pico_stdlib
	)
endif()
