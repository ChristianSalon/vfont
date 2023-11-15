# Find Wayland
#
#	Variables:
#
#		Wayland_FOUND
#		Wayland_client_LIBRARIES
#		Wayland_client_INCLUDE_DIRS
#		Wayland_PROTOCOLS_DIR
#		Wayland_SCANNER

# Find wayland-protocols directory
find_package(PkgConfig)
pkg_check_modules(Wayland_PROTOCOLS QUIET wayland-protocols)

if(PKG_CONFIG_FOUND AND Wayland_PROTOCOLS_FOUND)
	pkg_get_variable(Wayland_PROTOCOLS_DIR wayland-protocols pkgdatadir)
endif()

set(Wayland_PROTOCOLS_DIR "${Wayland_PROTOCOLS_DIR}")

# Find wayland-client
find_library(Wayland_client_LIBRARIES
	NAMES wayland-client
)
find_path(Wayland_client_INCLUDE_DIRS
	NAMES wayland-client.h
)

# Find wayland-scanner
find_program(Wayland_SCANNER
	NAMES wayland-scanner
)

if(Wayland_client_LIBRARIES AND Wayland_client_INCLUDE_DIRS AND Wayland_PROTOCOLS_DIR AND Wayland_SCANNER)
	set(Wayland_FOUND TRUE)
endif()
