SDK_FOLDER = "E:/Programming/source-sdk-2013/mp/src"
GARRYSMOD_MODULE_BASE_FOLDER = "../gmod-module-base"
SCANNING_FOLDER = "../scanning"
DETOURING_FOLDER = "../detouring"
SOURCE_FOLDER = "../source"
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
		objdir(PROJECT_FOLDER .. "/intermediate")
		targetdir(PROJECT_FOLDER .. "/release")

	filter({"configurations:Debug"})
		flags({"Symbols"})
		objdir(PROJECT_FOLDER .. "/intermediate")
		targetdir(PROJECT_FOLDER .. "/debug")

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
			SCANNING_FOLDER .. "/symbolfinder.cpp",
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
			libdirs({SDK_FOLDER .. "/lib/public", GARRYSMOD_MODULE_BASE_FOLDER})
			links({"ws2_32", "tier0", "tier1", "lua_shared"})
			targetsuffix("_win32")

			filter({"system:windows", "configurations:Debug"})
				linkoptions({"/NODEFAULTLIB:\"libcmt\""})

		filter("system:linux")
			defines({"POSIX", "GNUC", "_LINUX"})
			libdirs({SDK_FOLDER .. "/lib/public/linux32"})
			links({"dl", "tier0_srv"})
			prelinkcommands({
				"cp " .. SDK_FOLDER .. "/lib/public/linux32/libtier0.so ./libtier0_srv.so",
				"cp ../../" .. GARRYSMOD_MODULE_BASE_FOLDER .. "/lua_shared_srv.so ./"
			})
			linkoptions({
				SDK_FOLDER .. "/lib/public/linux32/tier1.a",
				"-Wl,-rpath,garrysmod/bin",
				"-l:lua_shared_srv.so"
			})
			buildoptions({"-std=c++11"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			libdirs({SDK_FOLDER .. "/lib/public/osx32", GARRYSMOD_MODULE_BASE_FOLDER})
			links({"dl", "tier0", "tier1", "lua_shared"})
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
			SCANNING_FOLDER .. "/symbolfinder.cpp",
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
		libdirs({GARRYSMOD_MODULE_BASE_FOLDER})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			libdirs({SDK_FOLDER .. "/lib/public", GARRYSMOD_MODULE_BASE_FOLDER})
			links({"ws2_32", "tier0", "tier1", "lua_shared"})
			targetsuffix("_win32")

			filter({"system:windows", "configurations:Debug"})
				linkoptions({"/NODEFAULTLIB:\"libcmt\""})

		filter("system:linux")
			defines({"POSIX", "GNUC", "_LINUX"})
			libdirs({SDK_FOLDER .. "/lib/public/linux32"})
			links({"dl", "tier0"})
			prelinkcommands({
				"cp -s " .. SDK_FOLDER .. "/lib/public/linux32/libtier0.so ./",
				"cp -s ../../" .. GARRYSMOD_MODULE_BASE_FOLDER .. "/lua_shared.so ./"
			})
			linkoptions({
				SDK_FOLDER .. "/lib/public/linux32/tier1.a",
				"-Wl,-rpath,garrysmod/bin",
				"-l:lua_shared.so"
			})
			buildoptions({"-std=c++11"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			libdirs({SDK_FOLDER .. "/lib/public/osx32", GARRYSMOD_MODULE_BASE_FOLDER})
			links({"dl", "tier0", "tier1", "lua_shared"})
			buildoptions({"-std=c++11"})
			targetsuffix("_mac")
