add_rules('mode.debug', 'mode.release')
set_project('RustCinder')
set_version('0.0.1')
set_xmakever('2.9.9')

set_warnings("all")
set_languages("cxx20")

set_allowedplats('linux')


SHARED_CONFIGS = {
    configs = {shared = true},
}


PUBLIC_CONFIGS = {
    public = true,
}

GLOBAL_CONFIG_FUNC = {
    ADD_MUDUO = function()
        local MUDUO_LINK_LIBS = {
                'muduo_protorpc',
                'muduo_protobuf_codec',
                'muduo_protorpc_wire',
                'z',
            }
        add_packages('muduo', PUBLIC_CONFIGS)
        add_links(MUDUO_LINK_LIBS)
    end,
    PROGRAMS = function()
        add_includedirs('include')
        add_files('src/**')
        add_deps('proto')
        add_packages('yaml-cpp', PUBLIC_CONFIGS)
        GLOBAL_CONFIG_FUNC['ADD_MUDUO']()
    end
}

add_requires('muduo', SHARED_CONFIGS)
add_requires('yaml-cpp', SHARED_CONFIGS)
add_requires('protobuf-cpp', SHARED_CONFIGS)
add_requires('protoc', SHARED_CONFIGS)

includes('proto')
includes('tests')

target('gateway')
    set_kind('binary')
    add_files('programs/gateway.cpp')
    GLOBAL_CONFIG_FUNC['PROGRAMS']()

target('rpc_client')
    set_kind('shared')
    add_files('src/client/rpc_client.cpp')
    GLOBAL_CONFIG_FUNC['PROGRAMS']()