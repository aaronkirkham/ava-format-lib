workspace "AvaFormatLib"
  configurations { "Debug", "Release" }
  location "projects"
  systemversion "latest"
  language "C++"
  targetdir "bin/%{cfg.buildcfg}"
  cppdialect "c++17"
  characterset "MBCS"
  architecture "x64"

  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"

  filter "configurations:Release"
    optimize "On"

group "Dependencies"
  project "zlib"
    kind "StaticLib"
    defines "DEF_WBITS=-15"
    files { "deps/zlib/*.c", "deps/zlib/*.h" }
    includedirs { "deps/zlib" }

project "AvaFormatLib"
  kind "StaticLib"
  files { "src/**.cpp", "include/**.h" }
  disablewarnings { "4200", "4267", "4334" }

project "UnitTests"
  kind "ConsoleApp"
  files "tests/**"
  dependson { "AvaFormatLib" }
  links { "out/%{cfg.buildcfg}/AvaFormatLib" }
  includedirs { "include" }
