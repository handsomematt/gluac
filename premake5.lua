solution "gluac"
	configurations { "Debug", "Release" }
	platforms { "x32", "x64" }

	language		"C++"
	characterset	"MBCS"
	location		"project"
	targetdir		"bin"

	filter "platforms:x32"
		architecture "x32"

	filter "platforms:x64"
		architecture "x64"

	project "gluac"
		kind	"ConsoleApp"
		targetname "gluac"
		flags { "Symbols", "NoEditAndContinue", "NoPCH", "StaticRuntime", "EnableSSE" }
		links "bootil_static"
		includedirs "Bootil/include"

        if os.is( "linux" ) then
                buildoptions { "-fPIC", "-pthread" }

                linkoptions { "-pthread" }
                links { "dl" }
        end

		files { "src/**.*" }

	include "Bootil/projects/bootil_premake5.lua"