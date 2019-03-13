newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
	value = "path to garrysmod_common directory"
})

include(assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory"))

CreateWorkspace({name = "sourcenet", abi_compatible = true})
	CreateProject({serverside = true})
		IncludeLuaShared()
		IncludeSDKCommon()
		IncludeSDKTier0()
		IncludeSDKTier1()
		IncludeScanning()
		IncludeDetouring()
		files({"../source/server/*.cpp", "../source/server/*.hpp"})
		links("LZMA")

	CreateProject({serverside = false})
		IncludeLuaShared()
		IncludeSDKCommon()
		IncludeSDKTier0()
		IncludeSDKTier1()
		IncludeScanning()
		IncludeDetouring()

	project("LZMA")
		language("C")
		kind("StaticLib")
		defines("_7ZIP_ST")
		files({
			"../source/lzma/*.h",
			"../source/lzma/*.c"
		})
		vpaths({
			["Header files"] = "../source/lzma/*.h",
			["Source files"] = "../source/lzma/*.c"
		})
