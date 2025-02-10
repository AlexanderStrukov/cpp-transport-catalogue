cmake_minimum_required(VERSION 3.12)

project(
	transport-catalogue
	VERSION 1.0
	LANGUAGES CXX
)

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h")

add_executable(TransportCatalogue ${SOURCES} ${HEADERS})
set_target_properties(
	TransportCatalogue PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(TransportCatalogue
	PRIVATE ./include
)