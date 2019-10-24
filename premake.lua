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

project "AvaFormatLib"
  kind "StaticLib"
  defines { "_CRT_SECURE_NO_WARNINGS", "DEF_WBITS=-15" }
  includedirs { "deps" }
  files { "src/**.cpp", "include/**.h" }
  files { "deps/zlib/*.c", "deps/zlib/*.h" }
  -- disablewarnings { "4200", "4267", "4334" }

project "UnitTests"
  kind "ConsoleApp"
  files "tests/**"
  dependson { "AvaFormatLib" }
  links { "AvaFormatLib" }
  includedirs { "include" }