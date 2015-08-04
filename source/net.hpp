#pragma once

#include <bitbuf.h>
#include <netadr.h>
#include <utlvector.h>
#include <inetchannel.h>

#if defined min || defined max

#undef min
#undef max

#endif

#include <cstdint>

// 0 == regular, 1 == file stream
#define MAX_STREAMS 2

// Flow control bytes per second limits
#define MAX_RATE		20000				
#define MIN_RATE		1000

// Default data rate
#define DEFAULT_RATE	10000

// NETWORKING INFO

// Number of bits to use for message type
#define NET_MESSAGE_BITS 6

// This is the packet payload without any header bytes (which are attached for actual sending)
#define	NET_MAX_PAYLOAD	80000

// This is the payload plus any header info (excluding UDP header)

// Packet header is:
//  4 bytes of outgoing seq
//  4 bytes of incoming seq
//  1 byte of padding and flags
//  and for each stream
// {
//  byte (on/off)
//  int (fragment id)
//  short (startpos)
//  short (length)
// }
#define HEADER_BYTES ( 8 + MAX_STREAMS * 9 )

// Pad this to next higher 16 byte boundary
// This is the largest packet that can come in/out over the wire, before processing the header
//  bytes will be stripped by the networking channel layer
#define	NET_MAX_MESSAGE	PAD_NUMBER( ( NET_MAX_PAYLOAD + HEADER_BYTES ), 16 )

// NOTE: Above should be 96k, may need tweaking

typedef enum
{
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

extern double net_time;

extern netadr_t net_local_adr;

extern qboolean net_noip;

#define MAX_FRAGMENT_SIZE 256

#define MAX_SUBCHANNELS 8

#define MAX_FILE_SIZE 0x3FFFFFF

#define FRAG_NORMAL_STREAM 0
#define FRAG_FILE_STREAM 1

typedef void *FileHandle_t;

typedef struct subchannel_s
{
	int32_t frag_ofs[MAX_STREAMS];	// offset of fragments to send (0x00, 0x04)
	int32_t frag_num[MAX_STREAMS];	// amount of fragments to send (0x08, 0x0C)
	int32_t sequence;				// outgoing seq. at time of sending (0x10)
	int32_t state;					// 0 = idle, 1 = needs sending, 2 = ready?, 3 = ??? (0x14)
	int32_t index;					// channel index (0-7) (0x18)
} subchannel_t;

typedef struct netpacket_s
{
	netadr_t from;				// sender IP (0x00)
	int32_t source;				// received source (0x0C)
	double received;			// received time (0x10)
	uint8_t *data;				// pointer to raw packet data (0x18)
	bf_read message;			// easy bitbuf data access (0x1C/28)
	int32_t size;				// size in bytes (0x40/64)
	int32_t wiresize;			// size in bytes before decompression (0x44/68)
	bool stream;				// was send as stream (0x48/72)
	struct netpacket_s *pNext;	// for internal use, should be NULL in public (0x4C/76)
} netpacket_t;

class CNetChan : public INetChannel
{
public:
	CNetChan( );
	//~CNetChan( );

	virtual const char  *GetName( ) const;	// get channel name
	virtual const char  *GetAddress( ) const; // get channel IP address as string
	virtual float		GetTime( ) const;	// current net time
	virtual float		GetTimeConnected( ) const;	// get connection time in seconds
	virtual int32_t		GetBufferSize( ) const;	// netchannel packet history size
	virtual int32_t		GetDataRate( ) const; // send data rate in byte/sec
	
	virtual bool		IsLoopback( ) const;	// true if loopback channel
	virtual bool		IsTimingOut( ) const = 0;	// true if timing out
	virtual bool		IsPlayback( ) const;	// true if demo playback

	virtual float		GetLatency( int32_t flow ) const = 0;	 // current latency (RTT), more accurate but jittering
	virtual float		GetAvgLatency( int32_t flow ) const = 0; // average packet latency in seconds
	virtual float		GetAvgLoss( int32_t flow ) const = 0;	 // avg packet loss[0..1]
	virtual float		GetAvgChoke( int32_t flow ) const = 0;	 // avg packet choke[0..1]
	virtual float		GetAvgData( int32_t flow ) const = 0;	 // data flow in bytes/sec
	virtual float		GetAvgPackets( int32_t flow ) const = 0; // avg packets/sec
	virtual int32_t		GetTotalData( int32_t flow ) const = 0;	 // total flow in/out in bytes
	virtual int32_t		GetSequenceNr( int32_t flow ) const = 0;	// last send seq number
	virtual bool		IsValidPacket( int32_t flow, int32_t frame_number ) const = 0; // true if packet was not lost/dropped/chocked/flushed
	virtual float		GetPacketTime( int32_t flow, int32_t frame_number ) const = 0; // time when packet was send
	virtual int32_t		GetPacketBytes( int32_t flow, int32_t frame_number, int32_t group ) const = 0; // group size of this packet
	virtual bool		GetStreamProgress( int32_t flow, int32_t *received, int32_t *total ) const = 0;  // TCP progress if transmitting
	virtual float		GetTimeSinceLastReceived( void ) const = 0;	// get time since last recieved packet in seconds
	virtual	float		GetCommandInterpolationAmount( int32_t flow, int32_t frame_number ) const = 0;
	virtual void		GetPacketResponseLatency( int32_t flow, int32_t frame_number, int32_t *pnLatencyMsecs, int32_t *pnChoke ) const = 0;
	virtual void		GetRemoteFramerate( float *pflFrameTime, float *pflFrameTimeStdDeviation ) const = 0;

	virtual float		GetTimeoutSeconds( ) const = 0;

	virtual	~CNetChan( ) { };

	virtual void		SetDataRate( float rate ) = 0;
	virtual bool		RegisterMessage( INetMessage &msg );
	virtual bool		StartStreaming( uint32_t challengeNr );
	virtual void		ResetStreaming( void );
	virtual void		SetTimeout( float seconds ) = 0;
	virtual void		SetDemoRecorder( IDemoRecorder *recorder );
	virtual void		SetChallengeNr( uint32_t chnr );

	virtual void		Reset( );
	virtual void		Clear( ) = 0;
	virtual void		Shutdown( const char *reason );

	virtual void		ProcessPlayback( ) = 0;
	virtual bool		ProcessStream( ) = 0;
	virtual void		ProcessPacket( netpacket_t *packet, bool bHasHeader ) = 0;

	virtual bool		SendNetMsg( INetMessage &msg, bool bForceReliable = false, bool bVoice = false );
	virtual bool		SendData( bf_write &msg, bool bReliable = true ) = 0;
	virtual bool		SendFile( const char *filename, uint32_t transferID );
	virtual void		DenyFile( const char *filename, uint32_t transferID );
	virtual void		RequestFile_OLD( const char *filename, uint32_t transferID );
	virtual void		SetChoked( );
	virtual int32_t		SendDatagram( bf_write *data );
	virtual bool		Transmit( bool onlyReliable = false ) = 0;

	virtual const netadr_t	&GetRemoteAddress( ) const;
	virtual INetChannelHandler *GetMsgHandler( ) const;
	virtual int32_t			GetDropNumber( ) const;
	virtual int32_t			GetSocket( ) const;
	virtual uint32_t		GetChallengeNr( ) const;
	virtual void			GetSequenceData( int32_t &nOutSequenceNr, int32_t &nInSequenceNr, int32_t &nOutSequenceNrAck );
	virtual void			SetSequenceData( int32_t nOutSequenceNr, int32_t nInSequenceNr, int32_t nOutSequenceNrAck );

	virtual void		UpdateMessageStats( int32_t msggroup, int32_t bits );
	virtual bool		CanPacket( ) const;
	virtual bool		IsOverflowed( ) const;
	virtual bool		IsTimedOut( ) const;
	virtual bool		HasPendingReliableData( );

	virtual void		SetFileTransmissionMode( bool bBackgroundMode );
	virtual void		SetCompressionMode( bool bUseCompression );
	virtual uint32_t RequestFile( const char *filename );

	virtual void		SetMaxBufferSize( bool bReliable, int32_t nBytes, bool bVoice = false );

	virtual bool		IsNull( ) const;
	virtual int32_t		GetNumBitsWritten( bool bReliable );
	virtual void		SetInterpolationAmount( float flInterpolationAmount );
	virtual void		SetRemoteFramerate( float flFrameTime, float flFrameTimeStdDeviation );

	// Max # of payload bytes before we must split/fragment the packet
	virtual void		SetMaxRoutablePayloadSize( int32_t nSplitSize );
	virtual int32_t		GetMaxRoutablePayloadSize( );

	typedef struct dataFragments_s
	{
		FileHandle_t hfile;			// file transfer handle (0x0)
		char filename[MAX_PATH];	// file transfer name (0x4)
		uint8_t *buffer;			// buffer transfer (0x108)
		uint32_t bytes;				// transfer bytes (0x10C)
		uint32_t bits;				// transfer bits (0x110)
		uint32_t transferid;		// file transfer ID (0x114)
		bool compressed;			// fragments are compressed (0x118)
		uint32_t actualsize;		// size when decompressed (0x11C)
		bool stream;				// send as stream (0x120)
		int32_t total;				// total amount of fragments to be sent (0x124)
		int32_t progress;			// amount of fragments sent so far (0x128)
		int32_t num;				// number of fragments to send when next possible (0x12C)
	} dataFragments_t;

	// Initialization
	void			Setup( netsrc_t socketnumber, netadr_t *adr, const char *nameid, INetChannelHandler *handler );

	// Fragment operations
	bool			IsFileInWaitingList( const char *filename );
	bool			IsValidForFileTransfer( const char *filename );
	bool			CreateFragmentsFromFile( const char *filename, int32_t stream, uint32_t transferID );
	bool			CreateFragmentsFromBuffer( bf_write *buf, int32_t stream );
	void			CompressFragments( );
	void			UncompressFragments( dataFragments_t *fragments );

	// Stream
	void			SendReliableViaStream( dataFragments_t *fragments );

	//
	bool			CheckReceivingList( int32_t stream );

	void			RemoveHeadInWaitingList( int32_t stream );

	// Subchannels
	int32_t			GetFreeSubChannel( );
	void			UpdateSubChannels( );
	bool			SendSubChannelData( bf_write &buf );
	bool			ReadSubChannelData( bf_read &buf, int32_t stream );

	// Process functions
	int32_t			ProcessPacketHeader( netpacket_t *packet );
	bool			ProcessControlMessage( int32_t cmd, bf_read &msg );
	bool			ProcessMessages( bf_read &buf );

	// Message lookup
	INetMessage		*FindMessage( int32_t type );

	// Packet flow
	void			FlowNewPacket( int32_t flow, int32_t incoming_sequence, int32_t outgoing_acknowledged, int32_t chokecount, int32_t dropcount, int32_t bytes );
	bool 			FlowUpdate( int32_t flow, int32_t bytes );

//private:
public:
	// States whether a message is being processed
	bool process_state;
	// The status of the last processed message
	bool fatal_error;

	// Sequencing variables
	//
	// Message we are sending to remote
	int32_t outgoing_sequence;
	// Increasing count of sequence numbers 
	int32_t incoming_sequence;
	// # of last outgoing message that has been ack'd.
	int32_t outgoing_acknowledged;
	// Maintained in UpdateSubChannels
	int32_t outgoing_reliable_value;
	// Maintained in ProcessPacket
	int32_t incoming_reliable_value;

	// Amount of choked packets since last dispatch
	int32_t chokecount;
	
	// Reliable message buffer.  We keep adding to it until reliable is acknowledged.  Then we clear it.
	bf_write reliabledata;
	CUtlMemory<uint8_t> reliablemem;

	// Unreliable message buffer.
	bf_write unreliabledata;
	CUtlMemory<uint8_t> unreliablemem;
	
	// Voice data buffer.
	bf_write voicedata;
	CUtlMemory<uint8_t> voicemem;

	// Socket number
	int32_t sock; //8C

	// Based on NET_ConnectSocket result
	int32_t statusmystery;

	// Max fragment bytes per packet (net_maxfragments)
	int32_t maxfragments;
	
	// Address this channel is talking to.
	netadr_t remote_address;

	// For timeouts.  Time last message was received.
	float last_received;
	// Time when channel was connected.
	double connect_time;

	// Bandwidth choke
	// Bytes per second
	int32_t rate;
	// If realtime > cleartime, free to send next packet
	double cleartime;

	// Waiting list of buffered fragments to go onto queue.
	// Multiple outgoing buffers can be queued in succession
	CUtlVector<dataFragments_t *> waitlist[MAX_STREAMS];
	
	dataFragments_t recvlist[MAX_STREAMS];

	subchannel_t subchannels[MAX_SUBCHANNELS]; //0 - 7

	int32_t transferid;
	
	bool backgroundmode;
	bool usecompression;
	
	bool tcpenabled;
	int32_t unkstream1;
	int32_t unkstream2;
	int32_t unkstream3;
	int32_t unkstream4;
	bool unkstream5;

	CUtlMemory<uint8_t> unknownmem;

	// Flow in there somewhere
	uint8_t unk16[8076];

	// 0x23D8 in Mac/Linux? Appears to be extra value in Windows somewhere, probably a class
	int32_t dropcount;
	
	// Channel name
	char name[32];

	int32_t challenge;
	
	// After this many seconds without receiving a packet from the server, the client will disconnect itself (cl_timeout)
	float timeout_seconds;
	
	INetChannelHandler *msghandler;

	CUtlVector<INetMessage *> netmessages;

	IDemoRecorder *demorecorder;
	
	int32_t numqueuedpackets;

	float interpolation;
	float frametime;
	float frametimestddeviation;

	// Max # of payload bytes before we must split/fragment the packet (net_maxroutable)
	int32_t splitsize;
	// a
	int32_t splitsequence;
};
