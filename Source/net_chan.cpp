//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "net.h"
#include "protocol.h"
#include "inetmsghandler.h"
#include "inetmessage.h"

#include <ctype.h>

bool COM_IsValidPath( const char *path )
{
	if ( path
		&& V_strlen( path ) > 0
		&& !V_strstr( path, "\\\\" )
		&& !V_strstr( path, ":" )
		&& !V_strstr( path, ".." ) )
		  return true;

	return false;
}

bool CNetChan::IsValidForFileTransfer( const char *filename )
{
	if ( ( !filename || !*filename ) || !COM_IsValidPath( filename ) )
		return false;

	const char *ext = V_strrchr( filename, '.' );
	
	if ( !ext || V_strlen( ext ) - 3 > 1 ) // Extension has to be 2 or 3 chars (1 fails due to unsigned/size_t data type, probably not intentional?)
		return false;
	
	const char *p = ext;
	
	while ( *p )
	{
		if ( isspace( *p ) ) // Ensure the extension has no spaces
			return false;
			
		p++;
	}

	if ( !V_strcasecmp( ext, ".cfg" )
		|| !V_strcasecmp( ext, ".lst" )
		|| !V_strcasecmp( ext, ".exe" )
		|| !V_strcasecmp( ext, ".vbs" )
		|| !V_strcasecmp( ext, ".com" )
		|| !V_strcasecmp( ext, ".bat" )
		|| !V_strcasecmp( ext, ".dll" )
		|| !V_strcasecmp( ext, ".ini" )
		|| !V_strcasecmp( ext, ".log" )
		|| !V_strcasecmp( ext, ".lua" )
		|| !V_strcasecmp( ext, ".vdf" )
		|| !V_strcasecmp( ext, ".smx" )
		|| !V_strcasecmp( ext, ".gcf" )
		|| !V_strcasecmp( ext, ".sys" ) )
		  return false;

	return true;
}

bool CNetChan::ProcessControlMessage( int cmd, bf_read &msg )
{
	switch ( cmd )
	{
	case net_NOP:
		{
			return true;
		}
	case net_Disconnect:
		{
			char reason[ 1024 ];
			msg.ReadString( reason, 1024 );

			msghandler->ConnectionClosing( reason );

			return false;
		}
	case net_File:
		{
			int transferID = msg.ReadUBitLong( 32 );

			char filename[ 1024 ];
			msg.ReadString( filename, 1024 );

			if ( msg.ReadOneBit() && IsValidForFileTransfer( filename ) )
				msghandler->FileRequested( filename, transferID );
			else
				msghandler->FileDenied( filename, transferID );

			return true;
		}
	default:
		{
			ConMsg( "Netchannel: received bad control cmd %i from %s.\n", cmd, remote_address.ToString() );

			return false;
		}
	}
}

INetMessage *CNetChan::FindMessage( int type )
{
	for ( int i = 0; i < netmessages.Count(); i++ )
	{
		INetMessage *msg = netmessages.Element( i );

		if ( msg->GetType() == type )
			return msg;
	}

	return NULL;
}