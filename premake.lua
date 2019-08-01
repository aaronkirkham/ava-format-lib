workspace "AvaFormatLib"
  configurations { "Debug", "Release" }
  location "out"
  systemversion "latest"
  language "C++"
  targetdir "out/%{cfg.buildcfg}"
  objdir "out"
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
  files { "src/**.cpp", "include/**.h" }
  disablewarnings { "4200", "4267", "4334" }