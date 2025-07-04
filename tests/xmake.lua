add_rules('mode.debug', 'mode.release')

add_requires('gtest', SHARED_CONFIGS)

local INCLUDE_DIR = '../include/'
local SRC_DIR = '../src/**'


target('gateway_test')
    set_kind("binary")
    set_languages("cxx20")
    add_includedirs(INCLUDE_DIR)
    add_files(SRC_DIR)
    add_files('./gateway/gateway_test.cpp')
    add_deps('proto')
    GLOBAL_CONFIG_FUNC['ADD_MUDUO']()

target('config_reader_test')
    add_files('./config/config_reader_test.cpp')
    add_packages('gtest', PUBLIC_CONFIGS)
    add_packages('yaml-cpp', PUBLIC_CONFIGS)

target('rpc_client_test')
    set_kind("binary")
    set_languages("cxx20")
    add_includedirs(INCLUDE_DIR)
    add_files(SRC_DIR)
    add_files('./client/rpc_client_test.cpp')
    add_packages('gtest', PUBLIC_CONFIGS)
    add_packages('yaml-cpp', PUBLIC_CONFIGS)
    add_deps('proto')
    add_cxflags("-g")
    -- add_runenvs('MUDUO_LOG_TRACE', '1')
    -- add_deps('rpc_client')
    -- add_links('rpc_client')
    GLOBAL_CONFIG_FUNC['ADD_MUDUO']()