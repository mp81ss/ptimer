#!lua

my_gcc_flags = {
    "-Wall", "-Wextra", "-pedantic", "-Wshadow", "-Wconversion", "-Wformat=2",
    "-Wstrict-prototypes", "-Wbad-function-cast"
}


newoption {
   trigger     = "doxygen_path",
   value       = "path",
   description = "Doxygen executable path"
}

newoption {
   trigger     = "pthread_include_path",
   value       = "path",
   description = "PThread include directory"
}

newoption {
   trigger     = "pthread_library_path",
   value       = "path",
   description = "PThread library directory"
}

newoption {
   trigger     = "pthread_library_name",
   value       = "name",
   description = "PThread library name"
}


solution "ptimer" -- no ansi due to usleep

    configurations { "debug", "release", "staticdebug", "staticrelease" }

    filter "configurations:*"
        flags "UndefinedIdentifiers"

    filter "configurations:*debug"
        defines { "DEBUG" }
        symbols "ON"

    filter { "toolset:gcc", "system:windows" }
        makesettings "CC=gcc"

    filter "configurations:*release"
        defines { "NDEBUG" }
        symbols "OFF"
        optimize "Speed"
        omitframepointer "ON"
        flags { "NoBufferSecurityCheck", "NoRuntimeChecks" }

    filter { "action:vs*", "configurations:static*" }
       staticruntime "on"

    filter { "action:vs*", "configurations:debug or release" }
       staticruntime "OFF"

    filter {
        "configurations:static*", "kind:StaticLib",
        "system:windows", "toolset:gcc or clang"
    }
        targetprefix "lib"
        targetextension ".a"

    filter {}


    local is_win = os.host() == "windows"
    local lib_path = nil
    local lib_name = nil

--    usage "ptimer_common"
    function ptimer_common()
        language "C"
    end

    function link_pthread()
        if lib_path then
            libdirs { lib_path }
        end
        if lib_name then
            links { lib_name }
        else
            links "pthread"
        end
    end

    if _OPTIONS["pthread_library_path"] then
        lib_path = _OPTIONS["pthread_library_path"]
    end
    if _OPTIONS["pthread_library_name"] then
        lib_name = _OPTIONS["pthread_library_name"]
    end


    project "ptimer"

        if _OPTIONS["pthread_include_path"] then
            includedirs { _OPTIONS["pthread_include_path"] }
        else
            if _ACTION:sub(0, 2) == "vs" then
                print("Warning: Your compiler may not have native support " ..
                      "for PThread library, \nplease consider the " ..
                      "--pthread_include_path option (premake5 --help)")
            end
        end

        filter "configurations:debug or release"
            kind "SharedLib"
            defines "PTIMER_SHARED_BUILD"
            if is_win and _ACTION:sub(0, 5) ~= "gmake" then
                link_pthread {}
            end
        filter "configurations:static*"
            kind "StaticLib"
        filter "toolset:gcc"
            buildoptions { my_gcc_flags, "-pthread" }
        filter "toolset:clang"
            buildoptions "-pthread"
        filter {}

        --uses { "ptimer_common" }
        ptimer_common {}

        files "ptimer.c"


    project "example"
        kind "ConsoleApp"
        cdialect "C89"
        ptimer_common {}
        dependson "ptimer"
        files "example.c"

        filter "toolset:gcc"
            buildoptions { my_gcc_flags }
        filter { "configurations:debug or release", "system:windows" }
            defines "PTIMER_SHARED"
        filter { "configurations:static*",
                 "system:windows",
                 "action:not gmake*"
        }
            link_pthread {}
        filter "system:not windows"
            link_pthread {}
        filter {}

        links "ptimer"


    project "doc"
        local dp = ""
        kind "Makefile"
        if _OPTIONS["doxygen_path"] then
            dp = _OPTIONS["doxygen_path"] .. '/'
        end
        buildcommands { dp .. "doxygen" }
        cleancommands "{RMDIR} doc"
