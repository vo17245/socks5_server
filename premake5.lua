workspace "socks5"
    configurations {"Debug","Release"}

project "socks5_server"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin-int/%{cfg.buildcfg}"
    files {"src/**.cpp","src/**.h"}
    removefiles { "src/echo_server.cpp"}
    includedirs
    {
        "/home/ubuntu/lib/libevent/include",
        "src",
        "vendor"
    }
    
    libdirs {"/home/ubuntu/lib/libevent/lib"}
    links { "event" }
    
    filter "configurations:Debug"
        defines {"CONFIG_DEBUG"}

        
    filter "configurations:Release"
        defines {"CONFIG_RELEASE"}
        optimize "On"

