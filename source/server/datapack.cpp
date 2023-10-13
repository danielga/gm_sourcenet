#include "datapack.hpp"
#include "protocol.hpp"

#include <GarrysMod/Lua/Helpers.hpp>
#include <GarrysMod/FunctionPointers.hpp>

#include <detouring/classproxy.hpp>

#include <networkstringtabledefs.h>
#include <iserver.h>
#include <iclient.h>
#include <eiface.h>
#include <LzmaLib.h>
#include <Sha256.h>

#include <vector>
#include <array>

class GModDataPack;

namespace DataPack
{
	static INetworkStringTable *client_lua_files = nullptr;

	class GModDataPackProxy : Detouring::ClassProxy<GModDataPack, GModDataPackProxy>
	{
	public:
		static void Initialize( GarrysMod::Lua::ILuaBase *LUA )
		{
			SendFileToClient_original = FunctionPointers::GModDataPack_SendFileToClient( );
			if( SendFileToClient_original == nullptr )
				LUA->ThrowError( "unable to find GModDataPack::SendFileToClient" );
		}

		void SendFileToClient( int client, int fileID )
		{
			if( !LuaHelpers::PushHookRun( global::lua, lua_file_hook_name ) )
			{
				Call( SendFileToClient_original, client, fileID );
				return;
			}

			global::lua->PushNumber( client );

			IClient *cl = global::server->GetClient( client );
			if( cl != nullptr )
				global::lua->PushString( cl->GetNetworkIDString( ) );
			else
				global::lua->PushNil( );

			global::lua->PushNumber( fileID );
			global::lua->PushString( client_lua_files->GetString( fileID ) );

			lua_receiving_client = client;

			bool dontcall = false;
			std::string substitute;
			if( LuaHelpers::CallHookRun( global::lua, 4, 1 ) )
			{
				if( global::lua->IsType( -1, GarrysMod::Lua::Type::BOOL ) )
					dontcall = global::lua->GetBool( -1 );

				global::lua->Pop( 1 );
			}

			lua_receiving_client = -1;

			if( !dontcall )
				Call( SendFileToClient_original, client, fileID );
		}

		static bool HookSendFileToClient( )
		{
			return Hook( SendFileToClient_original, &GModDataPackProxy::SendFileToClient );
		}

		static bool UnHookSendFileToClient( )
		{
			return UnHook( SendFileToClient_original );
		}

		LUA_FUNCTION_STATIC_MEMBER( EnableLuaFileValidation )
		{
			LUA->CheckType( 1, GarrysMod::Lua::Type::BOOL );
			LUA->PushBool( LUA->GetBool( 1 ) ? HookSendFileToClient( ) : UnHookSendFileToClient( ) );
			return 1;
		}

		static bool IsClientReceiving( int client )
		{
			return client == lua_receiving_client;
		}

	private:
		static FunctionPointers::GModDataPack_SendFileToClient_t SendFileToClient_original;

		static int lua_receiving_client;
		static const char lua_file_hook_name[];
	};

	FunctionPointers::GModDataPack_SendFileToClient_t
		GModDataPackProxy::SendFileToClient_original = nullptr;

	int GModDataPackProxy::lua_receiving_client = -1;
	const char GModDataPackProxy::lua_file_hook_name[] = "SendLuaFileToClient";

	inline size_t MaximumCompressedSize( const std::string &input )
	{
		const size_t input_size = input.size( ) + 1;
		return input_size + input_size / 3 + 128;
	}

	static std::vector<uint8_t> Compress( const std::string &input )
	{
		// The data is written:
		//	5 bytes:	props
		//	8 bytes:	uncompressed size
		//	the rest:	the actual data
		const size_t iInputLength = input.size( ) + 1;
		size_t props_size = LZMA_PROPS_SIZE;
		size_t iDestSize = iInputLength + iInputLength / 3 + 128;

		std::vector<uint8_t> output( iDestSize + LZMA_PROPS_SIZE + 8, 0 );

		const uint8_t *pInputData = reinterpret_cast<const uint8_t *>( input.c_str( ) );
		uint8_t *pPropStart = output.data( );
		uint8_t *pSizeStart = pPropStart + LZMA_PROPS_SIZE;
		uint8_t *pBodyStart = pSizeStart + 8;

		const int res = LzmaCompress(
			pBodyStart, &iDestSize, // Dest out
			pInputData, iInputLength, // Input in
			pPropStart, &props_size, // Props out
			5, // level [0-9]
			65536, // dict size ( ie 1 << 4 )
			3,
			0,
			2,
			32,
			2
		);

		if( props_size != LZMA_PROPS_SIZE )
			return { };

		if( res != SZ_OK )
			return { };

		// Write our 8 byte LE size
		pSizeStart[0] = iInputLength & 0xFF;
		pSizeStart[1] = ( iInputLength >> 8 ) & 0xFF;
		pSizeStart[2] = ( iInputLength >> 16 ) & 0xFF;
		pSizeStart[3] = ( iInputLength >> 24 ) & 0xFF;
		pSizeStart[4] = 0;
		pSizeStart[5] = 0;
		pSizeStart[6] = 0;
		pSizeStart[7] = 0;

		output.resize( iDestSize + LZMA_PROPS_SIZE + 8 );
		return output;
	}

	inline void SendLuaFile( int client, int fileID, const std::string &substitute, bool autorefresh )
	{
		const char *path = client_lua_files->GetString( fileID );

		// Type 1b + SHA256 32b + compressed substitute Xb + alignment 4b
		size_t min_buffer_size = 1 + 32 + MaximumCompressedSize( substitute ) + 4;

		if( autorefresh )
		{
			if( path == nullptr )
				return;

			// (File path + NUL) Xb + (SHA256 + compressed substitute) size 4b
			min_buffer_size += std::strlen( path ) + 1 + 4;
		}
		else
			// File ID 2b
			min_buffer_size += 2;

		std::vector<uint8_t> buffer( min_buffer_size, 0 );
		bf_write writer( "sourcenet SendLuaFile writer", buffer.data( ), static_cast<int>( buffer.size( ) ) );

		writer.WriteByte( autorefresh ? 1 : 4 );

		if( autorefresh )
			writer.WriteString( path );
		else
			writer.WriteWord( fileID );

		const std::vector<uint8_t> compressed_buffer = Compress( substitute );
		if( autorefresh )
			writer.WriteUBitLong( static_cast<uint32_t>( 32 + compressed_buffer.size( ) ), 32 );

		CSha256 sha256;
		Sha256_Init( &sha256 );
		Sha256_Update( &sha256, reinterpret_cast<const uint8_t *>( substitute.c_str( ) ), substitute.size( ) + 1 );
		std::array<uint8_t, 32> sha256_buffer { };
		Sha256_Final( &sha256, sha256_buffer.data( ) );
		writer.WriteBytes( sha256_buffer.data( ), static_cast<int>( sha256_buffer.size( ) ) );

		writer.WriteBytes( compressed_buffer.data( ), static_cast<int>( compressed_buffer.size( ) ) );

		global::engine_server->GMOD_SendToClient( client, writer.GetData( ), writer.GetNumBitsWritten( ) );
	}

	LUA_FUNCTION_STATIC( SendLuaFile )
	{
		LUA->CheckType( 1, GarrysMod::Lua::Type::NUMBER );
		LUA->CheckType( 2, GarrysMod::Lua::Type::NUMBER );
		LUA->CheckType( 3, GarrysMod::Lua::Type::STRING );

		bool autorefresh = false;
		if( LUA->IsType( 4, GarrysMod::Lua::Type::BOOL ) )
			autorefresh = LUA->GetBool( 4 );

		int clientID = static_cast<int>( LUA->GetNumber( 1 ) );
		if( clientID < 0 || clientID >= global::server->GetClientCount( ) )
		{
			LUA->PushNil( );
			LUA->PushString( "invalid client number" );
			return 2;
		}

		if( autorefresh && GModDataPackProxy::IsClientReceiving( clientID ) )
		{
			LUA->PushNil( );
			LUA->PushString( "unable to send Lua refreshes while a client is joining" );
			return 2;
		}

		SendLuaFile(
			clientID,
			static_cast<int>( LUA->GetNumber( 2 ) ),
			LUA->GetString( 3 ),
			autorefresh
		);
		LUA->PushBool( true );
		return 1;
	}

	void PreInitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		GModDataPackProxy::Initialize( LUA );

		INetworkStringTableContainer *networkstringtable =
			global::engine_loader.GetInterface<INetworkStringTableContainer>(
				INTERFACENAME_NETWORKSTRINGTABLESERVER
			);
		if( networkstringtable == nullptr )
			LUA->ThrowError( "unable to get INetworkStringTableContainer" );

		client_lua_files = networkstringtable->FindTable( "client_lua_files" );
		if( client_lua_files == nullptr )
			LUA->ThrowError( "missing \"client_lua_files\" string table" );
	}

	void Initialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		LUA->PushCFunction( GModDataPackProxy::EnableLuaFileValidation );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "EnableLuaFileValidation" );

		LUA->PushCFunction( SendLuaFile );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SendLuaFile" );
	}

	void Deinitialize( GarrysMod::Lua::ILuaBase *LUA )
	{
		GModDataPackProxy::UnHookSendFileToClient( );

		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "EnableLuaFileValidation" );

		LUA->PushNil( );
		LUA->SetField( GarrysMod::Lua::INDEX_GLOBAL, "SendLuaFile" );
	}
}
