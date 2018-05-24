solution "gluac"
	configurations { "Debug", "Release" }
	platforms { "x32" }

	language		"C++"
	characterset	"MBCS"
	location		"project"
	targetdir		"bin"
	architecture "x32"

	project "gluac"
		kind	"ConsoleApp"
		targetname "gluac"
		
		flags { "NoPCH" }
		symbols "On"
		editandcontinue "Off"
		staticruntime "On"
		vectorextensions "SSE"

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
                --linkoptions { "-pthread", "-Wl,-rpath=\\$$ORIGIN" }
                links { "dl" }
        end

		files { "src/**.*" }