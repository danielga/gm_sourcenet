#ifndef NET_H
#define NET_H

#include <bitbuf.h>
#include <netadr.h>
#include <utlvector.h>
#include <inetchannel.h>

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
	int frag_ofs[MAX_STREAMS];	// offset of fragments to send (0x00, 0x04)
	int frag_num[MAX_STREAMS];	// amount of fragments to send (0x08, 0x0C)
	int sequence;					// outgoing seq. at time of sending (0x10)
	int state;						// 0 = idle, 1 = needs sending, 2 = ready?, 3 = ??? (0x14)
	int index;						// channel index (0-7) (0x18)
} subchannel_t;

typedef struct netpacket_s
{
	netadr_t from;		// sender IP (0x00)
	int source;		// received source (0x0C)
	double received;	// received time (0x10)
	unsigned char *data;		// pointer to raw packet data (0x18)
	bf_read message;	// easy bitbuf data access (0x1C/28)
	int size;		// size in bytes (0x40/64)
	int wiresize;   // size in bytes before decompression (0x44/68)
	bool stream;		// was send as stream (0x48/72)
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
	virtual int			GetBufferSize( ) const;	// netchannel packet history size
	virtual int			GetDataRate( ) const; // send data rate in byte/sec
	
	virtual bool		IsLoopback( ) const;	// true if loopback channel
	virtual bool		IsTimingOut( ) const = 0;	// true if timing out
	virtual bool		IsPlayback( ) const;	// true if demo playback

	virtual float		GetLatency( int flow ) const = 0;	 // current latency (RTT), more accurate but jittering
	virtual float		GetAvgLatency( int flow ) const = 0; // average packet latency in seconds
	virtual float		GetAvgLoss( int flow ) const = 0;	 // avg packet loss[0..1]
	virtual float		GetAvgChoke( int flow ) const = 0;	 // avg packet choke[0..1]
	virtual float		GetAvgData( int flow ) const = 0;	 // data flow in bytes/sec
	virtual float		GetAvgPackets( int flow ) const = 0; // avg packets/sec
	virtual int			GetTotalData( int flow ) const = 0;	 // total flow in/out in bytes
	virtual int			GetSequenceNr( int flow ) const = 0;	// last send seq number
	virtual bool		IsValidPacket( int flow, int frame_number ) const = 0; // true if packet was not lost/dropped/chocked/flushed
	virtual float		GetPacketTime( int flow, int frame_number ) const = 0; // time when packet was send
	virtual int			GetPacketBytes( int flow, int frame_number, int group ) const = 0; // group size of this packet
	virtual bool		GetStreamProgress( int flow, int *received, int *total ) const = 0;  // TCP progress if transmitting
	virtual float		GetTimeSinceLastReceived( void ) const = 0;	// get time since last recieved packet in seconds
	virtual	float		GetCommandInterpolationAmount( int flow, int frame_number ) const = 0;
	virtual void		GetPacketResponseLatency( int flow, int frame_number, int *pnLatencyMsecs, int *pnChoke ) const = 0;
	virtual void		GetRemoteFramerate( float *pflFrameTime, float *pflFrameTimeStdDeviation ) const = 0;

	virtual float		GetTimeoutSeconds( ) const = 0;

	virtual	~CNetChan( ) { };

	virtual void		SetDataRate( float rate ) = 0;
	virtual bool		RegisterMessage( INetMessage &msg );
	virtual bool		StartStreaming( unsigned int challengeNr );
	virtual void		ResetStreaming( void );
	virtual void		SetTimeout( float seconds ) = 0;
	virtual void		SetDemoRecorder( IDemoRecorder *recorder );
	virtual void		SetChallengeNr( unsigned int chnr );

	virtual void		Reset( );
	virtual void		Clear( ) = 0;
	virtual void		Shutdown( const char *reason );

	virtual void		ProcessPlayback( ) = 0;
	virtual bool		ProcessStream( ) = 0;
	virtual void		ProcessPacket( netpacket_t *packet, bool bHasHeader ) = 0;

	virtual bool		SendNetMsg( INetMessage &msg, bool bForceReliable = false, bool bVoice = false );
	virtual bool		SendData( bf_write &msg, bool bReliable = true ) = 0;
	virtual bool		SendFile( const char *filename, unsigned int transferID );
	virtual void		DenyFile( const char *filename, unsigned int transferID );
	virtual void		RequestFile_OLD( const char *filename, unsigned int transferID );
	virtual void		SetChoked( );
	virtual int			SendDatagram( bf_write *data );
	virtual bool		Transmit( bool onlyReliable = false ) = 0;

	virtual const netadr_t	&GetRemoteAddress( ) const;
	virtual INetChannelHandler *GetMsgHandler( ) const;
	virtual int				GetDropNumber( ) const;
	virtual int				GetSocket( ) const;
	virtual unsigned int	GetChallengeNr( ) const;
	virtual void			GetSequenceData( int &nOutSequenceNr, int &nInSequenceNr, int &nOutSequenceNrAck );
	virtual void			SetSequenceData( int nOutSequenceNr, int nInSequenceNr, int nOutSequenceNrAck );

	virtual void		UpdateMessageStats( int msggroup, int bits );
	virtual bool		CanPacket( ) const;
	virtual bool		IsOverflowed( ) const;
	virtual bool		IsTimedOut( ) const;
	virtual bool		HasPendingReliableData( );

	virtual void		SetFileTransmissionMode( bool bBackgroundMode );
	virtual void		SetCompressionMode( bool bUseCompression );
	virtual unsigned int RequestFile( const char *filename );

	virtual void		SetMaxBufferSize( bool bReliable, int nBytes, bool bVoice = false );

	virtual bool		IsNull( ) const;
	virtual int			GetNumBitsWritten( bool bReliable );
	virtual void		SetInterpolationAmount( float flInterpolationAmount );
	virtual void		SetRemoteFramerate( float flFrameTime, float flFrameTimeStdDeviation );

	// Max # of payload bytes before we must split/fragment the packet
	virtual void		SetMaxRoutablePayloadSize( int nSplitSize );
	virtual int			GetMaxRoutablePayloadSize( );

	typedef struct dataFragments_s
	{
		FileHandle_t hfile;			// file transfer handle (0x0)
		char filename[MAX_PATH];	// file transfer name (0x4)
		unsigned char *buffer;		// buffer transfer (0x108)
		unsigned int bytes;			// transfer bytes (0x10C)
		unsigned int bits;			// transfer bits (0x110)
		unsigned int transferid;	// file transfer ID (0x114)
		bool compressed;			// fragments are compressed (0x118)
		unsigned int actualsize;	// size when decompressed (0x11C)
		bool stream;				// send as stream (0x120)
		int total;					// total amount of fragments to be sent (0x124)
		int progress;				// amount of fragments sent so far (0x128)
		int num;					// number of fragments to send when next possible (0x12C)
	} dataFragments_t;

	// Initialization
	void			Setup( netsrc_t socketnumber, netadr_t *adr, const char *nameid, INetChannelHandler *handler );

	// Fragment operations
	bool			IsFileInWaitingList( const char *filename );
	bool			IsValidForFileTransfer( const char *filename );
	bool			CreateFragmentsFromFile( const char *filename, int stream, unsigned int transferID );
	bool			CreateFragmentsFromBuffer( bf_write *buf, int stream );
	void			CompressFragments( );
	void			UncompressFragments( dataFragments_t *fragments );

	// Stream
	void			SendReliableViaStream( dataFragments_t *fragments );

	//
	bool			CheckReceivingList( int stream );

	void			RemoveHeadInWaitingList( int stream );

	// Subchannels
	int				GetFreeSubChannel( );
	void			UpdateSubChannels( );
	bool			SendSubChannelData( bf_write &buf );
	bool			ReadSubChannelData( bf_read &buf, int stream );

	// Process functions
	int				ProcessPacketHeader( netpacket_t *packet );
	bool			ProcessControlMessage( int cmd, bf_read &msg );
	bool			ProcessMessages( bf_read &buf );

	// Message lookup
	INetMessage		*FindMessage( int type );

	// Packet flow
	void			FlowNewPacket( int flow, int incoming_sequence, int outgoing_acknowledged, int chokecount, int dropcount, int bytes );
	bool 			FlowUpdate( int flow, int bytes );

//private:
public:
	// States whether a message is being processed
	bool process_state;
	// The status of the last processed message
	bool fatal_error;

	// Sequencing variables
	//
	// Message we are sending to remote
	int outgoing_sequence;
	// Increasing count of sequence numbers 
	int incoming_sequence;
	// # of last outgoing message that has been ack'd.
	int outgoing_acknowledged;
	// Maintained in UpdateSubChannels
	int outgoing_reliable_value;
	// Maintained in ProcessPacket
	int incoming_reliable_value;

	// Amount of choked packets since last dispatch
	int chokecount;
	
	// Reliable message buffer.  We keep adding to it until reliable is acknowledged.  Then we clear it.
	bf_write reliabledata;
	CUtlMemory<unsigned char> reliablemem;

	// Unreliable message buffer.
	bf_write unreliabledata;
	CUtlMemory<unsigned char> unreliablemem;
	
	// Voice data buffer.
	bf_write voicedata;
	CUtlMemory<unsigned char> voicemem;

	// Socket number
	int sock; //8C

	// Based on NET_ConnectSocket result
	int statusmystery;

	// Max fragment bytes per packet (net_maxfragments)
	int maxfragments;
	
	// Address this channel is talking to.
	netadr_t remote_address;

	// For timeouts.  Time last message was received.
	float last_received;
	// Time when channel was connected.
	double connect_time;

	// Bandwidth choke
	// Bytes per second
	int rate;
	// If realtime > cleartime, free to send next packet
	double cleartime;

	// Waiting list of buffered fragments to go onto queue.
	// Multiple outgoing buffers can be queued in succession
	CUtlVector<dataFragments_t *> waitlist[MAX_STREAMS];
	
	dataFragments_t recvlist[MAX_STREAMS];

	subchannel_t subchannels[MAX_SUBCHANNELS]; //0 - 7

	int transferid;
	
	bool backgroundmode;
	bool usecompression;
	
	bool tcpenabled;
	int unkstream1;
	int unkstream2;
	int unkstream3;
	int unkstream4;
	bool unkstream5;

	CUtlMemory<unsigned char> unknownmem;

	// Flow in there somewhere
	char unk16[8076];

	// 0x23D8 in Mac/Linux? Appears to be extra value in Windows somewhere, probably a class
	int dropcount;
	
	// Channel name
	char name[32];

	int challenge;
	
	// After this many seconds without receiving a packet from the server, the client will disconnect itself (cl_timeout)
	float timeout_seconds;
	
	INetChannelHandler *msghandler;

	CUtlVector<INetMessage *> netmessages;

	IDemoRecorder *demorecorder;
	
	int numqueuedpackets;

	float interpolation;
	float frametime;
	float frametimestddeviation;

	// Max # of payload bytes before we must split/fragment the packet (net_maxroutable)
	int splitsize;
	// a
	int splitsequence;
};

class CQueuedPacketSender : public CThread
{

public:

	virtual ~CQueuedPacketSender( );

	virtual bool	Start( unsigned nBytesStack = 0 );
	virtual int		Run( );

	virtual void	Setup( );
	virtual void	Shutdown( );
	virtual bool	IsRunning( );
	virtual void	ClearQueuedPacketsForChannel( INetChannel *netchan );
	virtual void	QueuePacket( INetChannel *netchan, int, const char *, int, const sockaddr *, int, unsigned int );
	virtual bool	HasQueuedPackets( const INetChannel *netchan ) const;

};

extern CQueuedPacketSender *g_pQueuedPacketSender;

#endif // NET_H