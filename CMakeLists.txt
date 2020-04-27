CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

PROJECT(dbcommands)

# disable debug define because code won't compile otherwise
add_compile_definitions(NDEBUG)

# set SYSTEM define
if(UNIX)
    add_compile_definitions(PLT_SYSTEM_LINUX=1)
elseif(WIN32)
    add_compile_definitions(PLT_SYSTEM_NT=1)
endif()

# add subfolders
add_subdirectory(acplt-ks)
add_subdirectory(fbs_dienste)

# define install rules
install(TARGETS fb_dbcommands
        RUNTIME DESTINATION bin)

if(${BUILD_DYNAMIC})
    install(TARGETS dbservices
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib)
    if(${BUILD_RPC})
        install(TARGETS rpc
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib)
    endif()
endif ()
