SDK_FOLDER = "E:/Programming/source-sdk-2013/mp/src"
GARRYSMOD_MODULE_BASE_FOLDER = "../gmod-module-base"
SCANNING_FOLDER = "../scanning"
DETOURING_FOLDER = "../detouring"
SOURCE_FOLDER = "../Source"
PROJECT_FOLDER = os.get() .. "/" .. _ACTION

solution("gm_sourcenet4")
	language("C++")
	location(PROJECT_FOLDER)
	warnings("Extra")
	flags({"NoPCH", "StaticRuntime"})
	platforms({"x86"})
	configurations({"Release", "Debug"})

	filter("platforms:x86")
		architecture("x32")

	filter("configurations:Release")
		optimize("On")
		vectorextensions("SSE2")
		objdir(PROJECT_FOLDER .. "/Intermediate")
		targetdir(PROJECT_FOLDER .. "/Release")

	filter({"configurations:Debug"})
		flags({"Symbols"})
		objdir(PROJECT_FOLDER .. "/Intermediate")
		targetdir(PROJECT_FOLDER .. "/Debug")

	project("gmsv_sourcenet4")
		kind("SharedLib")
		defines({
			"GMMODULE",
			"SOURCENET_SERVER",
			"SUPPRESS_INVALID_PARAMETER_NO_INFO"
		})
		includedirs({
			SOURCE_FOLDER,
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include",
			SCANNING_FOLDER,
			DETOURING_FOLDER,
			SDK_FOLDER .. "/common",
			SDK_FOLDER .. "/public",
			SDK_FOLDER .. "/public/tier0",
			SDK_FOLDER .. "/public/tier1"
		})
		files({
			SOURCE_FOLDER .. "/*.h",
			SOURCE_FOLDER .. "/*.hpp",
			SOURCE_FOLDER .. "/*.cpp",
			SCANNING_FOLDER .. "/SymbolFinder.cpp",
			DETOURING_FOLDER .. "/hde.cpp",
			SDK_FOLDER .. "/public/tier0/memoverride.cpp"
		})
		vpaths({
			["Header files"] = {
				SOURCE_FOLDER .. "/**.h",
				SOURCE_FOLDER .. "/**.hpp",
			},
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SCANNING_FOLDER .. "/**.cpp",
				DETOURING_FOLDER .. "/**.cpp",
				SDK_FOLDER .. "/**.cpp"
			}
		})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			libdirs({SDK_FOLDER .. "/lib/public"})
			links({"ws2_32", "tier0", "tier1"})
			targetsuffix("_win32")

			filter({"system:windows", "configurations:Debug"})
				linkoptions({"/NODEFAULTLIB:\"libcmt\""})

		filter("system:linux")
			defines({"POSIX", "GNUC", "_LINUX"})
			libdirs({SDK_FOLDER .. "/lib/public/linux32"})
			links({"dl", "tier0_srv"})
			linkoptions({SDK_FOLDER .. "/lib/public/linux32/tier1.a"})
			buildoptions({"-std=c++11"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			libdirs({SDK_FOLDER .. "/lib/public/osx32"})
			links({"dl", "tier0", "tier1"})
			buildoptions({"-std=c++11"})
			targetsuffix("_mac")

	project("gmcl_sourcenet4")
		kind("SharedLib")
		defines({
			"GMMODULE",
			"SOURCENET_CLIENT",
			"SUPPRESS_INVALID_PARAMETER_NO_INFO"
		})
		includedirs({
			SOURCE_FOLDER,
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include",
			SCANNING_FOLDER,
			DETOURING_FOLDER,
			SDK_FOLDER .. "/common",
			SDK_FOLDER .. "/public",
			SDK_FOLDER .. "/public/tier0",
			SDK_FOLDER .. "/public/tier1"
		})
		files({
			SOURCE_FOLDER .. "/*.h",
			SOURCE_FOLDER .. "/*.hpp",
			SOURCE_FOLDER .. "/*.cpp",
			SCANNING_FOLDER .. "/SymbolFinder.cpp",
			DETOURING_FOLDER .. "/hde.cpp",
			SDK_FOLDER .. "/public/tier0/memoverride.cpp"
		})
		vpaths({
			["Header files"] = {
				SOURCE_FOLDER .. "/**.h",
				SOURCE_FOLDER .. "/**.hpp",
			},
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SCANNING_FOLDER .. "/**.cpp",
				DETOURING_FOLDER .. "/**.cpp",
				SDK_FOLDER .. "/**.cpp"
			}
		})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			libdirs({SDK_FOLDER .. "/lib/public"})
			links({"ws2_32", "tier0", "tier1"})
			targetsuffix("_win32")

			filter({"system:windows", "configurations:Debug"})
				linkoptions({"/NODEFAULTLIB:\"libcmt\""})

		filter("system:linux")
			defines({"POSIX", "GNUC", "_LINUX"})
			libdirs({SDK_FOLDER .. "/lib/public/linux32"})
			links({"dl", "tier0"})
			linkoptions({SDK_FOLDER .. "/lib/public/linux32/tier1.a"})
			buildoptions({"-std=c++11"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			libdirs({SDK_FOLDER .. "/lib/public/osx32"})
			links({"dl", "tier0", "tier1"})
			buildoptions({"-std=c++11"})
			targetsuffix("_mac")
