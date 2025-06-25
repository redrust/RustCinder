
add_rules('mode.debug', 'mode.release')

target('proto')
    set_kind('object')
    add_files('**.proto', {proto_public = true})
    add_rules('protobuf.cpp')
    add_packages("protobuf-cpp", PUBLIC_CONFIGS)
    add_packages("protoc", {links = {}, linkdirs = {}}) -- 只是为了获取 host protoc, 不引入 linkflags
    set_policy('build.fence', true)
    add_cxxflags('-fPIC')
