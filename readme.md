# gm_sourcenet

A module for Garry's Mod that provides interfaces to many systems of VALVe's engine, based on [gm_sourcenet3][1].

## Info

Mac was not tested at all (sorry but I'm poor).

If stuff starts erroring or fails to work, be sure to check the correct line endings (\n and such) are present in the files for each OS.

This project requires [garrysmod_common][2], a framework to facilitate the creation of compilations files (Visual Studio, make, XCode, etc). Simply set the environment variable 'GARRYSMOD_COMMON' or the premake option 'gmcommon' to the path of your local copy of [garrysmod_common][2]. We also use [SourceSDK2013][3], so set the environment variable 'SOURCE_SDK' or the premake option 'sourcesdk' to the path of your local copy of [SourceSDK2013][3].


  [1]: http://christopherthorne.googlecode.com/svn/trunk/gm_sourcenet3
  [2]: https://github.com/danielga/garrysmod_common
  [3]: https://github.com/ValveSoftware/source-sdk-2013
