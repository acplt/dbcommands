
# packages needed for lexer/parser
find_package(BISON)
find_package(FLEX)

# configure library for ifb service libraries
add_library(dbservices STATIC
        ${CMAKE_CURRENT_BINARY_DIR}/fb_parser.c
        ${CMAKE_CURRENT_BINARY_DIR}/fb_scanner.c
        source/ifb_aufraeumen.cpp
        source/ifb_av.cpp
        source/ifb_cleandb.cpp
        source/ifb_compeval.cpp
        source/ifb_compproject.cpp
        source/ifb_createcomcon.cpp
        source/ifb_crobj.cpp
        source/ifb_dbsaveinstream.cpp
        #source/ifb_dbsaveinxml.cpp
        source/ifb_delfulltu.cpp
        source/ifb_delobj.cpp
        source/ifb_dir.cpp
        source/ifb_dupl.cpp
        source/ifb_fileup.cpp
        source/ifb_getcondata.cpp
        source/ifb_getportdata.cpp
        source/ifb_getserver.cpp
        source/ifb_importeval.cpp
        source/ifb_importproject.cpp
        source/ifb_logerror.cpp
        source/ifb_memfre.cpp
        source/ifb_readblockparam.cpp

        source/ifb_rename.cpp
        source/ifb_selectsave.cpp
        source/ifb_setpar.cpp
        source/ifb_tasklink.cpp
        source/ifb_updateeval.cpp
        source/ifb_updateproject.cpp
        source/ifb_varprops.cpp
        source/lts_cfnc.c)

target_include_directories(dbservices PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

# generarte fbd scanner code
BISON_TARGET(fb_parser source/fb_parser.y ${CMAKE_CURRENT_BINARY_DIR}/fb_parser.c COMPILE_FLAGS -t)
FLEX_TARGET(fb_scanner source/fb_scanner.lex  ${CMAKE_CURRENT_BINARY_DIR}/fb_scanner.c)
ADD_FLEX_BISON_DEPENDENCY(fb_scanner fb_parser)

target_link_libraries(dbservices kscln)


# configure fb_dbcommnads executable
add_executable(fb_dbcommands source/dbcommands.cpp source/templ_for_exec.cpp source/test_hist_templates.cpp)

target_link_libraries(fb_dbcommands plt ks kscln dbservices)
