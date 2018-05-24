//
// libtgvoip is free and unencumbered public domain software.
// For more information, see http://unlicense.org or the UNLICENSE file
// you should have received with this source code distribution.
//

#ifndef __VOIPCONTROLLER_H
#define __VOIPCONTROLLER_H

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#ifdef __APPLE__
#include <TargetConditionals.h>
#include "os/darwin/AudioUnitIO.h"
#endif
#include <stdint.h>
#include <vector>
#include <string>
#include <map>
#include "audio/AudioInput.h"
#include "BlockingQueue.h"
#include "BufferOutputStream.h"
#include "audio/AudioOutput.h"
#include "JitterBuffer.h"
#include "OpusDecoder.h"
#include "OpusEncoder.h"
#include "EchoCanceller.h"
#include "CongestionControl.h"
#include "NetworkSocket.h"
#include "BufferInputStream.h"

#define LIBTGVOIP_VERSION "2.0-alpha4"

#ifdef _WIN32
#undef GetCurrentTime
#endif

#define TGVOIP_PEER_CAP_GROUP_CALLS 1

struct voip_queued_packet_t{
	unsigned char type;
	unsigned char* data;
	size_t length;
	uint32_t seqs[16];
	double firstSentTime;
	double lastSentTime;
	double retryInterval;
	double timeout;
};
typedef struct voip_queued_packet_t voip_queued_packet_t;

struct voip_config_t{
	double init_timeout;
	double recv_timeout;
	int data_saving;
	char logFilePath[256];
	char statsDumpFilePath[256];

	bool enableAEC;
	bool enableNS;
	bool enableAGC;

	bool enableCallUpgrade;
};
typedef struct voip_config_t voip_config_t;

struct voip_stats_t{
	uint64_t bytesSentWifi;
	uint64_t bytesRecvdWifi;
	uint64_t bytesSentMobile;
	uint64_t bytesRecvdMobile;
};
typedef struct voip_stats_t voip_stats_t;

struct voip_crypto_functions_t{
	void (*rand_bytes)(uint8_t* buffer, size_t length);
	void (*sha1)(uint8_t* msg, size_t length, uint8_t* output);
	void (*sha256)(uint8_t* msg, size_t length, uint8_t* output);
	void (*aes_ige_encrypt)(uint8_t* in, uint8_t* out, size_t length, uint8_t* key, uint8_t* iv);
	void (*aes_ige_decrypt)(uint8_t* in, uint8_t* out, size_t length, uint8_t* key, uint8_t* iv);
	void (*aes_ctr_encrypt)(uint8_t* inout, size_t length, uint8_t* key, uint8_t* iv, uint8_t* ecount, uint32_t* num);
	void (*aes_cbc_encrypt)(uint8_t* in, uint8_t* out, size_t length, uint8_t* key, uint8_t* iv);
	void (*aes_cbc_decrypt)(uint8_t* in, uint8_t* out, size_t length, uint8_t* key, uint8_t* iv);
};
typedef struct voip_crypto_functions_t voip_crypto_functions_t;

#define SEQ_MAX 0xFFFFFFFF

inline bool seqgt(uint32_t s1, uint32_t s2){
	return ((s1>s2) && (s1-s2<=SEQ_MAX/2)) || ((s1<s2) && (s2-s1>SEQ_MAX/2));
}

namespace tgvoip{

	enum{
		PROXY_NONE=0,
		PROXY_SOCKS5,
		//PROXY_HTTP
	};

	enum{
		STATE_WAIT_INIT=1,
		STATE_WAIT_INIT_ACK,
		STATE_ESTABLISHED,
		STATE_FAILED,
		STATE_RECONNECTING
	};

	enum{
		ERROR_UNKNOWN=0,
		ERROR_INCOMPATIBLE,
		ERROR_TIMEOUT,
		ERROR_AUDIO_IO
	};

	enum{
		NET_TYPE_UNKNOWN=0,
		NET_TYPE_GPRS,
		NET_TYPE_EDGE,
		NET_TYPE_3G,
		NET_TYPE_HSPA,
		NET_TYPE_LTE,
		NET_TYPE_WIFI,
		NET_TYPE_ETHERNET,
		NET_TYPE_OTHER_HIGH_SPEED,
		NET_TYPE_OTHER_LOW_SPEED,
		NET_TYPE_DIALUP,
		NET_TYPE_OTHER_MOBILE
	};

	enum{
		DATA_SAVING_NEVER=0,
		DATA_SAVING_MOBILE,
		DATA_SAVING_ALWAYS
	};

	class Endpoint{
		friend class VoIPController;
		friend class VoIPGroupController;
	public:

		enum{
			TYPE_UDP_P2P_INET=1,
			TYPE_UDP_P2P_LAN,
			TYPE_UDP_RELAY,
			TYPE_TCP_RELAY
		};

		Endpoint(int64_t id, uint16_t port, IPv4Address& address, IPv6Address& v6address, char type, unsigned char* peerTag);
		Endpoint();
		int64_t id;
		uint16_t port;
		IPv4Address address;
		IPv6Address v6address;
		char type;
		unsigned char peerTag[16];

	private:
		double lastPingTime;
		uint32_t lastPingSeq;
		double rtts[6];
		double averageRTT;
		NetworkSocket* socket;
		int udpPongCount;
	};

	class AudioDevice{
	public:
		std::string id;
		std::string displayName;
	};

	class AudioOutputDevice : public AudioDevice{

	};

	class AudioInputDevice : public AudioDevice{

	};

	class VoIPController{
		friend class VoIPGroupController;
	public:
		VoIPController();
		virtual ~VoIPController();

		/**
		 * Set the initial endpoints (relays)
		 * @param endpoints Endpoints converted from phone.PhoneConnection TL objects
		 * @param allowP2p Whether p2p connectivity is allowed
		 */
		void SetRemoteEndpoints(std::vector<Endpoint> endpoints, bool allowP2p, int32_t connectionMaxLayer);
		/**
		 * Initialize and start all the internal threads
		 */
		void Start();
		/**
		 * Stop any internal threads. Don't call any other methods after this.
		 */
		void Stop();
		/**
		 * Initiate connection
		 */
		void Connect();
		Endpoint& GetRemoteEndpoint();
		/**
		 * Get the debug info string to be displayed in client UI
		 * @param buffer The buffer to put the string into
		 * @param len The length of the buffer
		 */
		virtual void GetDebugString(char* buffer, size_t len);
		/**
		 * Notify the library of network type change
		 * @param type The new network type
		 */
		virtual void SetNetworkType(int type);
		/**
		 * Get the average round-trip time for network packets
		 * @return
		 */
		double GetAverageRTT();
		static double GetCurrentTime();
		/**
		 * Use this field to store any of your context data associated with this call
		 */
		void* implData;
		/**
		 *
		 * @param mute
		 */
		virtual void SetMicMute(bool mute);
		/**
		 *
		 * @param key
		 * @param isOutgoing
		 */
		void SetEncryptionKey(char* key, bool isOutgoing);
		/**
		 *
		 * @param cfg
		 */
		void SetConfig(voip_config_t* cfg);
		float GetOutputLevel();
		void DebugCtl(int request, int param);
		/**
		 *
		 * @param stats
		 */
		void GetStats(voip_stats_t* stats);
		/**
		 *
		 * @return
		 */
		int64_t GetPreferredRelayID();
		/**
		 *
		 * @return
		 */
		int GetLastError();
		/**
		 *
		 */
		static voip_crypto_functions_t crypto;
		/**
		 *
		 * @return
		 */
		static const char* GetVersion();
		/**
		 *
		 * @return
		 */
		std::string GetDebugLog();
		/**
		 *
		 * @param buffer
		 */
		void GetDebugLog(char* buffer);
		size_t GetDebugLogLength();
		/**
		 *
		 * @return
		 */
		static std::vector<AudioInputDevice> EnumerateAudioInputs();
		/**
		 *
		 * @return
		 */
		static std::vector<AudioOutputDevice> EnumerateAudioOutputs();
		/**
		 *
		 * @param id
		 */
		void SetCurrentAudioInput(std::string id);
		/**
		 *
		 * @param id
		 */
		void SetCurrentAudioOutput(std::string id);
		/**
		 *
		 * @return
		 */
		std::string GetCurrentAudioInputID();
		/**
		 *
		 * @return
		 */
		std::string GetCurrentAudioOutputID();
		/**
		 * Set the proxy server to route the data through. Call this before connecting.
		 * @param protocol PROXY_NONE or PROXY_SOCKS5
		 * @param address IP address or domain name of the server
		 * @param port Port of the server
		 * @param username Username; empty string for anonymous
		 * @param password Password; empty string if none
		 */
		void SetProxy(int protocol, std::string address, uint16_t port, std::string username, std::string password);
		/**
		 * Get the number of signal bars to display in the client UI.
		 * @return the number of signal bars, from 1 to 4
		 */
		int GetSignalBarsCount();
		/**
		 * Enable or disable AGC (automatic gain control) on audio output. Should only be enabled on phones when the earpiece speaker is being used.
		 * The audio output will be louder with this on.
		 * AGC with speakerphone or other kinds of loud speakers has detrimental effects on some echo cancellation implementations.
		 * @param enabled I usually pick argument names to be self-explanatory
		 */
		void SetAudioOutputGainControlEnabled(bool enabled);
		/**
		 * Get the additional capabilities of the peer client app
		 * @return corresponding TGVOIP_PEER_CAP_* flags OR'ed together
		 */
		uint32_t GetPeerCapabilities();
		/**
		 * Send the peer the key for the group call to prepare this private call to an upgrade to a E2E group call.
		 * The peer must have the TGVOIP_PEER_CAP_GROUP_CALLS capability. After the peer acknowledges the key, Callbacks::groupCallKeySent will be called.
		 * @param key newly-generated group call key, must be exactly 265 bytes long
		 */
		void SendGroupCallKey(unsigned char* key);
		/**
		 * In an incoming call, request the peer to generate a new encryption key, send it to you and upgrade this call to a E2E group call.
		 */
		void RequestCallUpgrade();
		void SetEchoCancellationStrength(int strength);

		struct Callbacks{
			void (*connectionStateChanged)(VoIPController*, int);
			void (*signalBarCountChanged)(VoIPController*, int);
			void (*groupCallKeySent)(VoIPController*);
			void (*groupCallKeyReceived)(VoIPController*, unsigned char*);
			void (*upgradeToGroupCallRequested)(VoIPController*);
		};
		void SetCallbacks(Callbacks callbacks);

	protected:
		struct RecentOutgoingPacket{
			uint32_t seq;
			uint16_t id; // for group calls only
			double sendTime;
			double ackTime;
		};
		struct PendingOutgoingPacket{
			uint32_t seq;
			unsigned char type;
			size_t len;
			unsigned char* data;
			Endpoint* endpoint;
		};
		virtual void ProcessIncomingPacket(NetworkPacket& packet, Endpoint* srcEndpoint);
		virtual void WritePacketHeader(uint32_t seq, BufferOutputStream* s, unsigned char type, uint32_t length);
		virtual void SendPacket(unsigned char* data, size_t len, Endpoint* ep, PendingOutgoingPacket& srcPacket);
		virtual void SendInit();
		virtual void SendUdpPing(Endpoint* endpoint);
		virtual void SendRelayPings();
		virtual void OnAudioOutputReady();

	private:
		struct Stream{
			int32_t userID;
			unsigned char id;
			unsigned char type;
			uint32_t codec;
			bool enabled;
			uint16_t frameDuration;
			JitterBuffer* jitterBuffer;
			OpusDecoder* decoder;
			CallbackWrapper* callbackWrapper;
		};
		enum{
			UDP_UNKNOWN=0,
			UDP_PING_SENT,
			UDP_AVAILABLE,
			UDP_NOT_AVAILABLE,
			UDP_BAD
		};

		void RunRecvThread(void* arg);
		void RunSendThread(void* arg);
		void RunTickThread(void* arg);
		void HandleAudioInput(unsigned char* data, size_t len);
		void UpdateAudioBitrate();
		void SetState(int state);
		void UpdateAudioOutputState();
		void InitUDPProxy();
		void UpdateDataSavingState();
		void KDF(unsigned char* msgKey, size_t x, unsigned char* aesKey, unsigned char* aesIv);
		void KDF2(unsigned char* msgKey, size_t x, unsigned char* aesKey, unsigned char* aesIv);
		static size_t AudioInputCallback(unsigned char* data, size_t length, void* param);
		void SendPublicEndpointsRequest();
		void SendPublicEndpointsRequest(Endpoint& relay);
		Endpoint* GetEndpointByType(int type);
		void SendPacketReliably(unsigned char type, unsigned char* data, size_t len, double retryInterval, double timeout);
		uint32_t GenerateOutSeq();
		void LogDebugInfo();
		void ActuallySendPacket(NetworkPacket& pkt, Endpoint* ep);
		void StartAudio();
		int state;
		std::vector<Endpoint*> endpoints;
		Endpoint* currentEndpoint;
		Endpoint* preferredRelay;
		Endpoint* peerPreferredRelay;
		bool runReceiver;
		uint32_t seq;
		uint32_t lastRemoteSeq;
		uint32_t lastRemoteAckSeq;
		uint32_t lastSentSeq;
		std::vector<RecentOutgoingPacket> recentOutgoingPackets;
		double recvPacketTimes[32];
		uint32_t sendLossCountHistory[32];
		uint32_t audioTimestampIn;
		uint32_t audioTimestampOut;
		tgvoip::audio::AudioInput* audioInput;
		tgvoip::audio::AudioOutput* audioOutput;
		OpusEncoder* encoder;
		BlockingQueue<PendingOutgoingPacket>* sendQueue;
		EchoCanceller* echoCanceller;
		Mutex sendBufferMutex;
		Mutex endpointsMutex;
		bool stopping;
		bool audioOutStarted;
		Thread* recvThread;
		Thread* sendThread;
		Thread* tickThread;
		uint32_t packetsRecieved;
		uint32_t recvLossCount;
		uint32_t prevSendLossCount;
		uint32_t firstSentPing;
		double rttHistory[32];
		bool waitingForAcks;
		int networkType;
		int dontSendPackets;
		int lastError;
		bool micMuted;
		uint32_t maxBitrate;
		std::vector<Stream> outgoingStreams;
		std::vector<Stream> incomingStreams;
		unsigned char encryptionKey[256];
		unsigned char keyFingerprint[8];
		unsigned char callID[16];
		double stateChangeTime;
		bool waitingForRelayPeerInfo;
		bool allowP2p;
		bool dataSavingMode;
		bool dataSavingRequestedByPeer;
		std::string activeNetItfName;
		double publicEndpointsReqTime;
		std::vector<voip_queued_packet_t*> queuedPackets;
		Mutex queuedPacketsMutex;
		double connectionInitTime;
		double lastRecvPacketTime;
		voip_config_t config;
		int32_t peerVersion;
		CongestionControl* conctl;
		voip_stats_t stats;
		bool receivedInit;
		bool receivedInitAck;
		std::vector<std::string> debugLogs;
		bool isOutgoing;
		NetworkSocket* udpSocket;
		NetworkSocket* realUdpSocket;
		FILE* statsDump;
		std::string currentAudioInput;
		std::string currentAudioOutput;
		bool useTCP;
		bool useUDP;
		bool didAddTcpRelays;
		double setEstablishedAt;
		SocketSelectCanceller* selectCanceller;
		NetworkSocket* openingTcpSocket;
		unsigned char signalBarsHistory[4];

		BufferPool outgoingPacketsBufferPool;
		int udpConnectivityState;
		double lastUdpPingTime;
		int udpPingCount;
    	int echoCancellationStrength;

		int proxyProtocol;
		std::string proxyAddress;
		uint16_t proxyPort;
		std::string proxyUsername;
		std::string proxyPassword;
		IPv4Address* resolvedProxyAddress;

		int signalBarCount;

		AutomaticGainControl* outputAGC;
		bool outputAGCEnabled;
		uint32_t peerCapabilities;
		Callbacks callbacks;
		bool didReceiveGroupCallKey;
		bool didReceiveGroupCallKeyAck;
		bool didSendGroupCallKey;
		bool didSendUpgradeRequest;
		bool didInvokeUpdateCallback;

		int32_t connectionMaxLayer;
		bool useMTProto2;
		bool setCurrentEndpointToTCP;

		/*** server config values ***/
		uint32_t maxAudioBitrate;
		uint32_t maxAudioBitrateEDGE;
		uint32_t maxAudioBitrateGPRS;
		uint32_t maxAudioBitrateSaving;
		uint32_t initAudioBitrate;
		uint32_t initAudioBitrateEDGE;
		uint32_t initAudioBitrateGPRS;
		uint32_t initAudioBitrateSaving;
		uint32_t minAudioBitrate;
		uint32_t audioBitrateStepIncr;
		uint32_t audioBitrateStepDecr;
		double relaySwitchThreshold;
		double p2pToRelaySwitchThreshold;
		double relayToP2pSwitchThreshold;
		double reconnectingTimeout;

		/*** platform-specific things **/
#ifdef __APPLE__
		audio::AudioUnitIO* appleAudioIO;
#endif

	public:
#ifdef __APPLE__
		static double machTimebase;
		static uint64_t machTimestart;
#endif
#ifdef _WIN32
		static int64_t win32TimeScale;
		static bool didInitWin32TimeScale;
#endif
	};

	class VoIPGroupController : public VoIPController{
	public:
		VoIPGroupController(int32_t timeDifference);
		virtual ~VoIPGroupController();
		void SetGroupCallInfo(unsigned char* encryptionKey, unsigned char* reflectorGroupTag, unsigned char* reflectorSelfTag, unsigned char* reflectorSelfSecret,  unsigned char* reflectorSelfTagHash, int32_t selfUserID, IPv4Address reflectorAddress, IPv6Address reflectorAddressV6, uint16_t reflectorPort);
		void AddGroupCallParticipant(int32_t userID, unsigned char* memberTagHash, unsigned char* serializedStreams, size_t streamsLength);
		void RemoveGroupCallParticipant(int32_t userID);
		float GetParticipantAudioLevel(int32_t userID);
		virtual void SetMicMute(bool mute);
		void SetParticipantVolume(int32_t userID, float volume);
		void SetParticipantStreams(int32_t userID, unsigned char* serializedStreams, size_t length);
		static size_t GetInitialStreams(unsigned char* buf, size_t size);

		struct Callbacks : public VoIPController::Callbacks{
			void (*updateStreams)(VoIPGroupController*, unsigned char*, size_t);
			void (*participantAudioStateChanged)(VoIPGroupController*, int32_t, bool);

		};
		void SetCallbacks(Callbacks callbacks);
		virtual void GetDebugString(char* buffer, size_t length);
		virtual void SetNetworkType(int type);
	protected:
		virtual void ProcessIncomingPacket(NetworkPacket& packet, Endpoint* srcEndpoint);
		virtual void SendInit();
		virtual void SendUdpPing(Endpoint* endpoint);
		virtual void SendRelayPings();
		virtual void SendPacket(unsigned char* data, size_t len, Endpoint* ep, PendingOutgoingPacket& srcPacket);
		virtual void WritePacketHeader(uint32_t seq, BufferOutputStream* s, unsigned char type, uint32_t length);
		virtual void OnAudioOutputReady();
	private:
		int32_t GetCurrentUnixtime();
		std::vector<Stream> DeserializeStreams(BufferInputStream& in);
		void SendRecentPacketsRequest();
		void SendSpecialReflectorRequest(unsigned char* data, size_t len);
		void SerializeAndUpdateOutgoingStreams();
		struct GroupCallParticipant{
			int32_t userID;
			unsigned char memberTagHash[32];
			std::vector<Stream> streams;
			AudioLevelMeter* levelMeter;
		};
		std::vector<GroupCallParticipant> participants;
		unsigned char reflectorSelfTag[16];
		unsigned char reflectorSelfSecret[16];
		unsigned char reflectorSelfTagHash[32];
		int32_t userSelfID;
		Endpoint* groupReflector;
		AudioMixer* audioMixer;
		AudioLevelMeter selfLevelMeter;
		Callbacks groupCallbacks;
		struct PacketIdMapping{
			uint32_t seq;
			uint16_t id;
			double ackTime;
		};
		std::vector<PacketIdMapping> recentSentPackets;
		Mutex sentPacketsMutex;
		Mutex participantsMutex;
		int32_t timeDifference;
	};

};

#endif
