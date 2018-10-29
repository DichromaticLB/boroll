cmake_minimum_required (VERSION 3.10)
project (Boroll)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
PKG_CHECK_MODULES(GTKMM gtkmm-3.0)

FIND_PACKAGE(BISON)
BISON_TARGET(keyv syntaxTools/keyv.y ${CMAKE_CURRENT_BINARY_DIR}/keyv.cpp)
SET(BISON_keyvf_COMPILE_FLAGS -d)

FIND_PACKAGE(FLEX REQUIRED)
FLEX_TARGET(keyvf syntaxTools/keyvscanner.l ${CMAKE_CURRENT_BINARY_DIR}/keyvf.cpp)

add_executable(boroll Boroll.cpp  
	 BoAppLoop.cpp 
         Bofiguration.cpp 
         CurlWrap.cpp 
         ShiftingFrame.cpp 
         XMLPostFetch.cpp 
         pugixml-1.9/pugixml.cpp
	 ${BISON_keyv_OUTPUTS}
	 ${FLEX_keyvf_OUTPUTS})

target_link_libraries(boroll PRIVATE ${CURL_LIBRARIES}  ${GTKMM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(boroll PRIVATE pugixml-1.9 ${CURL_INCLUDE_DIR} ${GTKMM_INCLUDE_DIRS})
target_compile_definitions(boroll PRIVATE _GTKMAINBOROLL)