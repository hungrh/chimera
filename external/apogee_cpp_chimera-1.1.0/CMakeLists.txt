cmake_minimum_required(VERSION 2.8)

project(APOGEE_CHIMERA)

ADD_DEFINITIONS(-c -g -fPIC )

#TODO : fazer funcionar com boost_python

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/include
	${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/include/libapogee-2.1/apogee
#	/opt/libs/boost-1.52.0/include
#	/usr/include/python2.6/
)

link_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/lib
#	/opt/libs/boost-1.52.0/lib
) 

add_library (
	apogee_chimera SHARED
	src/ApogeeAltaManager.cpp
	src/apgSampleCmn.cpp
)

target_link_libraries(
	apogee_chimera 
	ccd
	cfitsio
	apogee
	wcs
	usb-1.0
	curl
	tcl
#    boost_python
#    python2.6
)

SET_TARGET_PROPERTIES(apogee_chimera
                      PROPERTIES
                      VERSION 1.1.0
                      SOVERSION 1)

file(COPY src/ApogeeWrapper.py DESTINATION .)
file(COPY test/ApogeeWrapperTest.py DESTINATION .)

#install(TARGETS apogee_chimera DESTINATION /usr/bin)
