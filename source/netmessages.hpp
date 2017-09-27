#pragma once

#include <main.hpp>
#include <inetmessage.h>
#include <utlvector.h>
#include <engine/iserverplugin.h>
#include <checksum_crc.h>
#include <protocol.hpp>

class INetChannel;
class INetMessageHandler;

namespace NetMessages
{
	class CNetMessage : public INetMessage
	{
	public:
		CNetMessage( );

		virtual void SetNetChannel( INetChannel *netchan );
		virtual void SetReliable( bool state );

		virtual bool Process( );
		virtual bool ReadFromBuffer( bf_read &buffer );
		virtual bool WriteToBuffer( bf_write &buffer );

		virtual bool IsReliable( ) const;

		virtual int32_t GetType( ) const;
		virtual int32_t GetGroup( ) const;
		virtual const char *GetName( ) const;

		virtual INetChannel *GetNetChannel( ) const;

		virtual const char *ToString( ) const;

		void InstallVTable( void **vtable );

		void **GetVTable( );

	protected:
		bool m_bReliable;
		INetChannel *m_NetChannel;
		INetMessageHandler *m_pMessageHandler;
	};

	class NET_Tick : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t Tick;
		float HostFrameTime;
		float HostFrameTimeStdDeviation;
	};

	class NET_StringCmd : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		NET_StringCmd( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		const char *Command;

	private:
		char CommandBuffer[1024];
	};

	class NET_SetConVar : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		typedef struct cvar_s
		{
			char name[260];
			char value[260];
		} cvar_t;

		CUtlVector<cvar_t> ConVars;
	};

	class NET_SignonState : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t SignonState;
		int32_t SpawnCount;
	};

	class SVC_Print : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_Print( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		const char *Text;

	private:
		char TextBuffer[2048];
	};

	class SVC_ServerInfo : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_ServerInfo( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t Protocol;
		int32_t ServerCount;
		bool Dedicated;
		bool HLTV;
		char OS;
		CRC32_t ClientCRC;
		uint8_t MapMD5[16];
		int32_t MaxClients;
		int32_t MaxClasses;
		int32_t PlayerSlot;
		float TickInterval;
		const char *GameDir;
		const char *MapName;
		const char *SkyName;
		const char *HostName;
		const char *LoadingURL;
		const char *Gamemode;

	private:
		char GameDirBuffer[260];
		char MapNameBuffer[260];
		char SkyNameBuffer[260];
		char HostNameBuffer[260];
		char LoadingURLBuffer[260];
		char GameModeBuffer[260];
	};

	class SVC_SendTable : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		bool NeedsDecoder;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_ClassInfo : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		typedef struct class_s
		{
			int32_t classID;
			char datatablename[256];
			char classname[256];
		} class_t;

		bool CreateOnClient;
		CUtlVector<class_t> Classes;
		int32_t NumServerClasses;
	};

	class SVC_SetPause : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		bool Paused;
	};

	class SVC_CreateStringTable : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_CreateStringTable( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		const char *TableName;
		int32_t MaxEntries;
		int32_t NumEntries;
		bool UserDataFixedSize;
		int32_t UserDataSize;
		int32_t UserDataSizeBits;
		bool IsFilenames;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;

	private:
		char TableNameBuffer[256];
	};

	class SVC_UpdateStringTable : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t TableID;
		int32_t ChangedEntries;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_VoiceInit : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_VoiceInit( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		const char *VoiceCodec;
		int32_t Quality;

	private:
		char VoiceCodecBuffer[260];
	};

	class SVC_VoiceData : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_VoiceData( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t Client;
		bool Proximity;
		int32_t Length;
		uint64_t XUID;
		bf_read DataIn;
		void *DataOut;
	};

	class SVC_Sounds : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		bool ReliableSound;
		int32_t NumSounds;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_SetView : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t EntityIndex;
	};

	class SVC_FixAngle : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		bool Relative;
		QAngle Angle;
	};

	class SVC_CrosshairAngle : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		QAngle Angle;
	};

	class SVC_BSPDecal : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		Vector Pos;
		int32_t DecalTextureIndex;
		int32_t EntityIndex;
		int32_t ModelIndex;
		bool LowPriority;
	};

	class SVC_UserMessage : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t MsgType;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_EntityMessage : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t EntityIndex;
		int32_t ClassID;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_GameEvent : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_PacketEntities : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t MaxEntries;
		int32_t UpdatedEntries;
		bool Delta;
		bool UpdateBaseline;
		int32_t Baseline;
		int32_t DeltaFrom;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_TempEntities : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t NumEntries;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_Prefetch : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		enum
		{
			SOUND = 0,
		};

		uint16_t SoundType;
		uint16_t SoundIndex;
	};

	class SVC_Menu : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_Menu( );

		~SVC_Menu( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		KeyValues *MenuKeyValues;
		DIALOG_TYPE DialogType;
		int32_t Length;
	};

	class SVC_GameEventList : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t NumEvents;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class SVC_GetCvarValue : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_GetCvarValue( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		QueryCvarCookie_t Cookie;
		const char *CvarName;

	private:
		char CvarNameBuffer[256];
	};

	class SVC_CmdKeyValues : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		SVC_CmdKeyValues( );

		~SVC_CmdKeyValues( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		KeyValues *CmdKeyValues;
	};

	class SVC_GMod_ServerToClient : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		bf_read Data;
	};

	class CLC_ClientInfo : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		CRC32_t SendTableCRC;
		int32_t ServerCount;
		bool HLTV;
		uint32_t FriendsID;
		char FriendsName[MAX_PLAYER_NAME_LENGTH];
		CRC32_t CustomFiles[MAX_CUSTOM_FILES];
	};

	class CLC_Move : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t BackupCommands;
		int32_t NewCommands;
		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
	};

	class CLC_VoiceData : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t Length;
		bf_read DataIn;
		bf_write DataOut;
		uint64_t XUID;
	};

	class CLC_BaselineAck : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		int32_t BaselineTick;
		int32_t BaselineNr;
	};

	class CLC_ListenEvents : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		CBitVec<MAX_EVENT_NUMBER> EventArray;
	};

	class CLC_RespondCvarValue : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		CLC_RespondCvarValue( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		QueryCvarCookie_t Cookie;
		const char *CvarName;
		const char *CvarValue;
		EQueryCvarValueStatus StatusCode;

	private:
		char CvarNameBuffer[256];
		char CvarValueBuffer[256];
	};

	class CLC_FileCRCCheck : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		char PathID[260];
		char Filename[260];
		CRC32_t CRC;
	};

	class CLC_CmdKeyValues : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		CLC_CmdKeyValues( );
		~CLC_CmdKeyValues( );

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		KeyValues *CmdKeyValues;
	};

	class CLC_FileMD5Check : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		char PathID[260];
		char Filename[260];
		uint8_t MD5[16];
	};

	class CLC_GMod_ClientToServer : public CNetMessage
	{
	public:
		static const char Name[];
		static const char LuaName[];
		static const int32_t Type;

		static void SetupLua( GarrysMod::Lua::ILuaBase *LUA );

		bf_read Data;
	};
}
