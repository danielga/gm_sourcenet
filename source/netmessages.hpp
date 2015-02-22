#pragma once

#include <main.hpp>
#include <inetchannelinfo.h>
#include <inetmessage.h>
#include <igameevents.h>
#include <protocol.hpp>
#include <unordered_map>

#define SETUP_NETMESSAGE( name ) \
	name( ) \
	{ \
		memset( this, 0, sizeof( name ) ); \
	} \
	virtual void Destroy( ) \
	{ \
		CNetMessage::Destroy( ); \
		memset( this, 0, sizeof( name ) ); \
	} \
	static CNetMessage *Create( void **vtable ) \
	{ \
		CNetMessage *msg = new name; \
		msg->InstallVTable( vtable ); \
		return msg; \
	}

namespace NetMessage
{

class CNetMessage : public INetMessage
{
public:
	virtual ~CNetMessage( )
	{ };

	virtual void SetNetChannel( INetChannel *netchan )
	{
		m_NetChannel = netchan;
	}

	virtual void SetReliable( bool state )
	{
		m_bReliable = state;
	}

	virtual bool Process( )
	{
		return false;
	}

	virtual bool ReadFromBuffer( bf_read &buffer )
	{
		return false;
	}

	virtual bool WriteToBuffer( bf_write &buffer )
	{
		return false;
	}

	virtual bool IsReliable( ) const
	{
		return m_bReliable;
	}

	virtual int32_t GetType( ) const
	{
		return -1;
	}

	virtual int32_t GetGroup( ) const
	{
		return INetChannelInfo::GENERIC;
	}

	virtual const char *GetName( ) const
	{
		return nullptr;
	}

	virtual INetChannel *GetNetChannel( ) const
	{
		return m_NetChannel;
	}

	virtual const char *ToString( ) const
	{
		return nullptr;
	}

	virtual void Destroy( )
	{
		this->~CNetMessage( );
	}

	inline void InstallVTable( void **vtable )
	{
		*reinterpret_cast<void ***>( this ) = vtable;
	}

	inline void **GetVTable( )
	{
		return *reinterpret_cast<void ***>( this );
	}

protected:
	bool m_bReliable;
	INetChannel *m_NetChannel;
};

class NET_Tick : public CNetMessage
{
public:
	SETUP_NETMESSAGE( NET_Tick );

	int32_t m_nTick;
	float m_flHostFrameTime;
	float m_flHostFrameTimeStdDeviation;
};

class NET_StringCmd : public CNetMessage
{
public:
	SETUP_NETMESSAGE( NET_StringCmd );

	const char *m_szCommand;

private:
	char m_szCommandBuffer[1024];
};

class NET_SetConVar : public CNetMessage
{
public:
	SETUP_NETMESSAGE( NET_SetConVar );

	typedef struct cvar_s
	{
		char name[260];
		char value[260];
	} cvar_t;

	CUtlVector<cvar_t> m_ConVars;
};

class NET_SignonState : public CNetMessage
{
public:
	SETUP_NETMESSAGE( NET_SignonState );

	int32_t m_nSignonState;
	int32_t m_nSpawnCount;
};

class SVC_Print : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_Print );

	const char *m_szText;

private:
	char m_szTextBuffer[2048];
};

class SVC_ServerInfo : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_ServerInfo );

	int16_t m_nProtocol;
	int32_t m_nServerCount;
	bool m_bIsHLTV;
	bool m_bIsDedicated;
	CRC32_t m_nClientCRC;
	int16_t m_nMaxClasses;
	uint8_t m_nMapMD5[16];
	uint8_t m_nPlayerSlot;
	uint8_t m_nMaxClients;
	float m_fTickInterval;
	char m_cOS;
	const char *m_szGameDir;
	const char *m_szMapName;
	const char *m_szSkyName;
	const char *m_szHostName;
	const char *m_szLoadingURL;
	const char *m_szGameMode;

private:
	char m_szGameDirBuffer[260];
	char m_szMapNameBuffer[260];
	char m_szSkyNameBuffer[260];
	char m_szHostNameBuffer[260];
	char m_szLoadingURLBuffer[260];
	char m_szGameModeBuffer[260];
};

class SVC_SendTable : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_SendTable );

	bool m_bNeedsDecoder;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_ClassInfo : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_ClassInfo );

	typedef struct class_s
	{
		int32_t classID;
		char datatablename[256];
		char classname[256];
	} class_t;

	bool m_bCreateOnClient;
	CUtlVector<class_t> m_Classes;			
	int32_t m_nNumServerClasses;
};

class SVC_SetPause : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_SetPause );

	bool m_bPaused;
};

class SVC_CreateStringTable : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_CreateStringTable );

	const char *m_szTableName;
	int32_t m_nMaxEntries;
	int32_t m_nNumEntries;
	bool m_bUserDataFixedSize;
	int32_t m_nUserDataSize;
	int32_t m_nUserDataSizeBits;
	bool m_bIsFilenames;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;

private:
	char m_szTableNameBuffer[256];
};

class SVC_UpdateStringTable : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_UpdateStringTable );

	int32_t m_nTableID;
	int32_t m_nChangedEntries;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_VoiceInit : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_VoiceInit );

	const char *m_szVoiceCodec;
	int32_t m_nQuality;

private:
	char m_szVoiceCodecBuffer[260];
};

class SVC_VoiceData : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_VoiceData );

	int32_t m_nFromClient;
	bool m_bProximity;
	int32_t m_nLength;
	uint64_t m_xuid;

	bf_read m_DataIn;
	void *m_DataOut;
};

class SVC_Sounds : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_Sounds );

	bool m_bReliableSound;
	int32_t m_nNumSounds;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_SetView : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_SetView );

	int32_t m_nEntityIndex;
};

class SVC_FixAngle: public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_FixAngle );

	bool m_bRelative; 
	QAngle m_Angle;
};

class SVC_CrosshairAngle : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_CrosshairAngle );

	QAngle m_Angle;
};

class SVC_BSPDecal : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_BSPDecal );

	Vector m_Pos;
	int32_t m_nDecalTextureIndex;
	int32_t m_nEntityIndex;
	int32_t m_nModelIndex;
	bool m_bLowPriority;
};

class SVC_UserMessage: public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_UserMessage );

	int32_t m_nMsgType;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_EntityMessage : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_EntityMessage );

	int32_t m_nEntityIndex;
	int32_t m_nClassID;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_GameEvent : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_GameEvent );

	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_PacketEntities : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_PacketEntities );

	int32_t m_nMaxEntries;
	int32_t m_nUpdatedEntries;
	bool m_bIsDelta;	
	bool m_bUpdateBaseline;
	int32_t m_nBaseline;
	int32_t m_nDeltaFrom;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_TempEntities : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_TempEntities );

	int32_t m_nNumEntries;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_Prefetch : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_Prefetch );

	enum
	{
		SOUND = 0,
	};

	uint16_t m_fType;
	uint16_t m_nSoundIndex;
};

class SVC_Menu : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_Menu );

	KeyValues *m_MenuKeyValues;
	DIALOG_TYPE m_Type;
	int32_t m_iLength;
};

class SVC_GameEventList : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_GameEventList );

	int32_t m_nNumEvents;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class SVC_GetCvarValue : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_GetCvarValue );

	QueryCvarCookie_t m_iCookie;
	const char *m_szCvarName;

private:
	char m_szCvarNameBuffer[256];
};

class SVC_CmdKeyValues : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_CmdKeyValues );

	KeyValues *m_CmdKeyValues;
};

class SVC_GMod_ServerToClient : public CNetMessage
{
public:
	SETUP_NETMESSAGE( SVC_GMod_ServerToClient );

	bf_read m_Data;
};

class CLC_ClientInfo : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_ClientInfo );

	CRC32_t m_nSendTableCRC;
	int32_t m_nServerCount;
	bool m_bIsHLTV;
	uint32_t m_nFriendsID;
	char m_FriendsName[MAX_PLAYER_NAME_LENGTH];
	CRC32_t m_nCustomFiles[MAX_CUSTOM_FILES];
};

class CLC_Move : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_Move );

	int32_t m_nBackupCommands;
	int32_t m_nNewCommands;
	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
};

class CLC_VoiceData : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_VoiceData );

	int32_t m_nLength;
	bf_read m_DataIn;
	bf_write m_DataOut;
	uint64_t m_xuid;
};

class CLC_BaselineAck : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_BaselineAck );

	int32_t m_nBaselineTick;
	int32_t m_nBaselineNr;
};

class CLC_ListenEvents : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_ListenEvents );

	CBitVec<MAX_EVENT_NUMBER> m_EventArray;
};

class CLC_RespondCvarValue : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_RespondCvarValue );

	QueryCvarCookie_t m_iCookie;
	
	const char *m_szCvarName;
	const char *m_szCvarValue;

	EQueryCvarValueStatus m_eStatusCode;

private:
	char m_szCvarNameBuffer[256];
	char m_szCvarValueBuffer[256];
};

class CLC_FileCRCCheck : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_FileCRCCheck );

	char m_szPathID[260];
	char m_szFilename[260];
	CRC32_t m_CRC;
};

class CLC_CmdKeyValues : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_CmdKeyValues );

	KeyValues *m_CmdKeyValues;
};

class CLC_FileMD5Check : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_FileMD5Check );

	char m_szPathID[260];
	char m_szFilename[260];
	uint8_t m_MD5[16];
};

class CLC_GMod_ClientToServer : public CNetMessage
{
public:
	SETUP_NETMESSAGE( CLC_GMod_ClientToServer );

	bf_read m_Data;
};

typedef CNetMessage *( *NetMessageCreator )( void **vtable );
static std::unordered_map<int32_t, NetMessageCreator> netmessages_creators = {
	{ net_Tick, NET_Tick::Create },
	{ net_StringCmd, NET_StringCmd::Create },
	{ net_SetConVar, NET_StringCmd::Create },
	{ net_SignonState, NET_StringCmd::Create },
	{ svc_Print, NET_StringCmd::Create },
	{ svc_ServerInfo, SVC_ServerInfo::Create },
	{ svc_SendTable, SVC_SendTable::Create },
	{ svc_ClassInfo, SVC_ClassInfo::Create },
	{ svc_SetPause, SVC_SetPause::Create },
	{ svc_CreateStringTable, SVC_CreateStringTable::Create },
	{ svc_UpdateStringTable, SVC_UpdateStringTable::Create },
	{ svc_VoiceInit, SVC_VoiceInit::Create },
	{ svc_VoiceData, SVC_VoiceData::Create },
	{ svc_Sounds, SVC_Sounds::Create },
	{ svc_SetView, SVC_SetView::Create },
	{ svc_FixAngle, SVC_FixAngle::Create },
	{ svc_CrosshairAngle, SVC_CrosshairAngle::Create },
	{ svc_BSPDecal, SVC_BSPDecal::Create },
	{ svc_UserMessage, SVC_UserMessage::Create },
	{ svc_EntityMessage, SVC_EntityMessage::Create },
	{ svc_GameEvent, SVC_GameEvent::Create },
	{ svc_PacketEntities, SVC_PacketEntities::Create },
	{ svc_TempEntities, SVC_TempEntities::Create },
	{ svc_Prefetch, SVC_Prefetch::Create },
	{ svc_Menu, SVC_Menu::Create },
	{ svc_GameEventList, SVC_GameEventList::Create },
	{ svc_GetCvarValue, SVC_GetCvarValue::Create },
	{ svc_CmdKeyValues, SVC_CmdKeyValues::Create },
	{ svc_GMod_ServerToClient, SVC_GMod_ServerToClient::Create },
	{ clc_ClientInfo, CLC_ClientInfo::Create },
	{ clc_Move, CLC_Move::Create },
	{ clc_VoiceData, CLC_VoiceData::Create },
	{ clc_BaselineAck, CLC_BaselineAck::Create },
	{ clc_ListenEvents, CLC_ListenEvents::Create },
	{ clc_RespondCvarValue, CLC_RespondCvarValue::Create },
	{ clc_FileCRCCheck, CLC_FileCRCCheck::Create },
	{ clc_CmdKeyValues, CLC_CmdKeyValues::Create },
	{ clc_FileMD5Check, CLC_FileMD5Check::Create },
	{ clc_GMod_ClientToServer, CLC_GMod_ClientToServer::Create }
};

static CNetMessage *CreateNetMessage( int32_t type, void **vtable )
{
	auto it = netmessages_creators.find( type );
	if( it != netmessages_creators.end( ) )
		return ( *it ).second( vtable );

	return nullptr;
}

}