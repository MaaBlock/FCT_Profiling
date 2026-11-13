function(set_utf8_for_target target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE
                /source-charset:utf-8
                /execution-charset:utf-8
        )
    else()
        target_compile_options(${target_name} PRIVATE
                -finput-charset=UTF-8
                -fexec-charset=UTF-8
        )
    endif()
endfunction()
