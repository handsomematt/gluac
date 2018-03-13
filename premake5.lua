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
		
		flags { "NoPCH" }
		symbols "On"
		editandcontinue "Off"
		staticruntime "On"
		vectorextensions "SSE"

		links "bootil_static"
		includedirs "Bootil/include"

		includedirs "scanning"
		files {
			"scanning/*.hpp",
			"scanning/*.cpp"
		}
		vpaths {
			["Symbol Scanning/Headers/*"] = "scanning/*.hpp",
			["Symbol Scanning/Sources/*"] = "scanning/*.cpp"
		}

        if os.istarget( "linux" ) then
                buildoptions { "-fPIC", "-pthread" }

                linkoptions { "-pthread" }
                links { "dl" }
        end

		files { "src/**.*" }

	project "bootil_static"
		uuid ( "AB8E7B19-A70C-4767-88DE-F02160167C2E" )
		defines { "BOOTIL_COMPILE_STATIC", "BOOST_ALL_NO_LIB" }
		files { "Bootil/src/**.cpp", "Bootil/include/**.h", "Bootil/src/**.c", "Bootil/src/**.cc" }
		kind "StaticLib"
		targetname( "bootil_static" )

		flags { "NoPCH" }
		symbols "On"
		editandcontinue "Off"
		staticruntime "On"
		vectorextensions "SSE"

		includedirs { "Bootil/include/", "Bootil/src/3rdParty/" }
		
		if os.istarget( "linux" ) or os.istarget( "macosx" ) then
			buildoptions { "-fPIC" }
		end
		
	