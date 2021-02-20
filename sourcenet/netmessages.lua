require("sourcenet")

-- Debug ConVar
local sourcenet_netmessage_info = CreateConVar("sourcenet_netmessage_info", "0")

local function log2(val)
	return math.ceil(math.log(val) / math.log(2))
end

-- Engine definitions
NET_MESSAGE_BITS = 6
NUM_NEW_COMMAND_BITS = 4
NUM_BACKUP_COMMAND_BITS = 3
MAX_TABLES_BITS = log2(32)
MAX_USERMESSAGE_BITS = 11
MAX_ENTITYMESSAGE_BITS = 11
MAX_SERVER_CLASS_BITS = 9
MAX_EDICT_BITS = 13

function SourceNetMsg(msg)
	if sourcenet_netmessage_info:GetInt() ~= 0 then
		Msg("[snmi] " .. msg .. "\n")
	end
end

NET_MESSAGES = {
	NET = {
		[net_NOP] = { -- 0
			__tostring = function()
				return "net_NOP"
			end,
			__index = {
				GetType = function()
					return net_NOP
				end,
				GetName = function()
					return "net_NOP"
				end,
				ReadFromBuffer = function(self)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(net_NOP, NET_MESSAGE_BITS)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
				end
			}
		},

		[net_Disconnect] = { -- 1
			__tostring = function(self)
				if not self.initialized then
					return "net_Disconnect"
				end

				return "net_Disconnect: " .. self.reason
			end,
			__index = {
				GetType = function()
					return net_Disconnect
				end,
				GetName = function()
					return "net_Disconnect"
				end,
				ReadFromBuffer = function(self, buffer)
					self.reason = buffer:ReadString()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(net_Disconnect, NET_MESSAGE_BITS)
					buffer:WriteString(self.reason)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.reason = nil
				end
			}
		},

		[net_File] = { -- 2
			__tostring = function(self)
				if not self.initialized then
					return "net_File"
				end

				if not self.requested then
					return string.format("net_File: transferid = %i, requested = false", self.transferid)
				end

				return string.format("net_File: transferid = %i, requested = true, requesttype = %s, fileid = %i", self.transferid, self.requesttype, self.fileid)
			end,
			__index = {
				GetType = function()
					return net_File
				end,
				GetName = function()
					return "net_File"
				end,
				ReadFromBuffer = function(self, buffer)
					self.transferid = buffer:ReadUInt(32)
					self.requested = buffer:ReadBit() == 1
					if not self.requested then
						self.initialized = true
						return
					end

					self.requesttype = buffer:ReadBit() == 1
					self.fileid = buffer:ReadUInt(32)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(net_File, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.transferid, 32)
					buffer:WriteBit(self.requested and 1 or 0)

					if not self.requested then
						return true
					end

					buffer:WriteBit(self.requesttype and 1 or 0)
					buffer:WriteUInt(self.fileid, 32)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.transferid = nil
					self.requested = nil
					self.requesttype = nil
					self.fileid = nil
				end
			}
		},

		[net_Tick] = { -- 3
			__tostring = function(self)
				if not self.initialized then
					return "net_Tick"
				end

				return string.format("net_Tick: tick = %i, hostframetime = %i, hostframetimedeviation = %i", self.tick, self.hostframetime, self.hostframetimedeviation)
			end,
			__index = {
				GetType = function()
					return net_Tick
				end,
				GetName = function()
					return "net_Tick"
				end,
				ReadFromBuffer = function(self, buffer)
					self.tick = buffer:ReadLong()
					self.hostframetime = buffer:ReadUInt(16)
					self.hostframetimedeviation = buffer:ReadInt(16)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(net_Tick, NET_MESSAGE_BITS)
					buffer:WriteLong(self.tick)
					buffer:WriteUInt(self.hostframetime, 16)
					buffer:WriteInt(self.hostframetimedeviation, 16)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.tick = nil
					self.hostframetime = nil
					self.hostframetimedeviation = nil
				end
			}
		},

		[net_StringCmd] = { -- 4
			__tostring = function(self)
				if not self.initialized then
					return "net_StringCmd"
				end

				return string.format("net_StringCmd: len = %d, cmd = %s", #self.cmd, self.cmd)
			end,
			__index = {
				GetType = function()
					return net_StringCmd
				end,
				GetName = function()
					return "net_StringCmd"
				end,
				ReadFromBuffer = function(self, buffer)
					self.cmd = buffer:ReadString()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(net_StringCmd, NET_MESSAGE_BITS)
					buffer:WriteString(self.cmd)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.cmd = nil
				end
			}
		},

		[net_SetConVar] = { -- 5
			__tostring = function(self)
				if not self.initialized then
					return "net_SetConVar"
				end

				local values
				for i = 1, #self.cvars do
					local cvar = self.cvars[i]
					if values then
						values = string.format("%s, %s = %s", values, cvar.name, cvar.value)
					else
						values = string.format("%s = %s", cvar.name, cvar.value)
					end
				end

				return "net_SetConVar: " .. values
			end,
			__index = {
				GetType = function()
					return net_SetConVar
				end,
				GetName = function()
					return "net_SetConVar"
				end,
				ReadFromBuffer = function(self, buffer)
					local count = buffer:ReadByte()
					self.cvars = {}
					for i = 1, count do
						local cvarname = buffer:ReadString()
						local cvarvalue = buffer:ReadString()
						self.cvars[i] = {name = cvarname, value = cvarvalue}
					end

					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(net_SetConVar, NET_MESSAGE_BITS)
					buffer:WriteByte(#self.cvars)

					for i = 1, #self.cvars do
						local cvar = self.cvars[i]
						buffer:WriteString(cvar.name)
						buffer:WriteString(cvar.value)
					end

					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.cvars = nil
				end
			}
		},

		[net_SignonState] = { -- 6
			__tostring = function(self)
				if not self.initialized then
					return "net_SignonState"
				end

				return string.format("net_SignonState: state = %i, spawncount = %i", self.state, self.spawncount)
			end,
			__index = {
				GetType = function()
					return net_SignonState
				end,
				GetName = function()
					return "net_SignonState"
				end,
				ReadFromBuffer = function(self, buffer)
					self.state = buffer:ReadByte()
					self.spawncount = buffer:ReadLong()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(net_SignonState, NET_MESSAGE_BITS)
					buffer:WriteByte(self.state)
					buffer:WriteLong(self.spawncount)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.state = nil
					self.spawncount = nil
				end
			}
		}
	},

	CLC = {
		[clc_ClientInfo] = { -- 8
			__tostring = function(self)
				if not self.initialized then
					return "clc_ClientInfo"
				end

				return string.format("clc_ClientInfo: spawncount = %i, sendtablecrc = %i, ishltv = %s, friendsid = %i, guid = %s", self.spawncount, self.sendtablecrc, self.ishltv, self.friendsid, self.guid)
			end,
			__index = {
				GetType = function()
					return clc_ClientInfo
				end,
				GetName = function()
					return "clc_ClientInfo"
				end,
				ReadFromBuffer = function(self, buffer)
					self.spawncount = buffer:ReadLong()
					self.sendtablecrc = buffer:ReadLong()
					self.ishltv = buffer:ReadBit() == 1
					self.friendsid = buffer:ReadLong()
					self.guid = buffer:ReadString()
					self.customfiles = {}
					for i = 1, MAX_CUSTOM_FILES do
						self.customfiles[i] = buffer:ReadBit() == 1 and buffer:ReadUInt(32) or false
					end

					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_ClientInfo, NET_MESSAGE_BITS)
					buffer:WriteLong(self.spawncount)
					buffer:WriteLong(self.sendtablecrc)
					buffer:WriteBit(self.ishltv and 1 or 0)
					buffer:WriteLong(self.friendsid)
					buffer:WriteString(self.guid)

					for i = 1, MAX_CUSTOM_FILES do
						local filecrc = self.customfiles[i]
						buffer:WriteBit(filecrc ~= false and 1 or 0)
						if filecrc then
							buffer:WriteUInt(filecrc, 32)
						end
					end

					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.spawncount = nil
					self.sendtablecrc = nil
					self.ishltv = nil
					self.friendsid = nil
					self.guid = nil
					self.customfiles = nil
				end
			}
		},

		[clc_Move] = { -- 9
			__tostring = function(self)
				if not self.initialized then
					return "clc_Move"
				end

				return string.format("clc_Move: new = %i, backup = %i, bits = %i", self.new, self.backup, self.bits)
			end,
			__index = {
				GetType = function()
					return clc_Move
				end,
				GetName = function()
					return "clc_Move"
				end,
				ReadFromBuffer = function(self, buffer)
					self.new = buffer:ReadUInt(NUM_NEW_COMMAND_BITS)
					self.backup = buffer:ReadUInt(NUM_BACKUP_COMMAND_BITS)
					self.bits = buffer:ReadWord()
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_Move, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.new, NUM_NEW_COMMAND_BITS)
					buffer:WriteUInt(self.backup, NUM_BACKUP_COMMAND_BITS)
					buffer:WriteWord(self.bits)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.new = nil
					self.backup = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[clc_VoiceData] = { -- 10
			__tostring = function(self)
				if not self.initialized then
					return "clc_VoiceData"
				end

				return "clc_VoiceData: bits = " .. self.bits
			end,
			__index = {
				GetType = function()
					return clc_VoiceData
				end,
				GetName = function()
					return "clc_VoiceData"
				end,
				ReadFromBuffer = function(self, buffer)
					self.bits = buffer:ReadWord()
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_VoiceData, NET_MESSAGE_BITS)
					buffer:WriteWord(self.bits)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[clc_BaselineAck] = { -- 11
			__tostring = function(self)
				if not self.initialized then
					return "clc_BaselineAck"
				end

				return string.format("clc_BaselineAck: tick = %i, num = %i", self.tick, self.num)
			end,
			__index = {
				GetType = function()
					return clc_BaselineAck
				end,
				GetName = function()
					return "clc_BaselineAck"
				end,
				ReadFromBuffer = function(self, buffer)
					self.tick = buffer:ReadLong()
					self.num = buffer:ReadUInt(1)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_BaselineAck, NET_MESSAGE_BITS)
					buffer:WriteLong(self.tick)
					buffer:WriteUInt(self.num, 1)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.tick = nil
					self.num = nil
				end
			}
		},

		[clc_ListenEvents] = { -- 12
			__tostring = function()
				return "clc_ListenEvents"
			end,
			__index = {
				GetType = function()
					return clc_ListenEvents
				end,
				GetName = function()
					return "clc_ListenEvents"
				end,
				ReadFromBuffer = function(self, buffer)
					self.events = {}
					for i = 1, 16 do
						self.events[i] = buffer:ReadUInt(32)
					end

					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_ListenEvents, NET_MESSAGE_BITS)

					for i = 1, 16 do
						buffer:WriteUInt(self.events[i], 32)
					end

					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.events = nil
				end
			}
		},

		[clc_RespondCvarValue] = { -- 13
			__tostring = function(self)
				if not self.initialized then
					return "clc_RespondCvarValue"
				end

				return string.format("clc_RespondCvarValue: cookie = %i, status = %i, cvarname = %s, cvarvalue = %s", self.cookie, self.status, self.cvarname, self.cvarvalue)
			end,
			__index = {
				GetType = function()
					return clc_RespondCvarValue
				end,
				GetName = function()
					return "clc_RespondCvarValue"
				end,
				ReadFromBuffer = function(self, buffer)
					self.cookie = buffer:ReadInt(32)
					self.status = buffer:ReadInt(4)
					self.cvarname = buffer:ReadString()
					self.cvarvalue = buffer:ReadString()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_RespondCvarValue, NET_MESSAGE_BITS)
					buffer:WriteInt(self.cookie, 32)
					buffer:WriteInt(self.status, 4)
					buffer:WriteString(self.cvarname)
					buffer:WriteString(self.cvarvalue)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.cookie = nil
					self.status = nil
					self.cvarname = nil
					self.cvarvalue = nil
				end
			}
		},

		[clc_FileCRCCheck] = { -- 14
			__tostring = function(self)
				if not self.initialized then
					return "clc_FileCRCCheck"
				end

				return string.format("clc_FileCRCCheck: %s, %s, %s, %i, %s, %i", self.reserved, self.gamepath, self.pathid, self.prefixid, self.filename, self.crc)
			end,
			__index = {
				GetType = function()
					return clc_FileCRCCheck
				end,
				GetName = function()
					return "clc_FileCRCCheck"
				end,
				ReadFromBuffer = function(self, buffer)
					self.reserved = buffer:ReadBit() == 1
					self.gamepath = buffer:ReadUInt(2)
					self.pathid = "commonpath"
					if self.gamepath == 0 then
						self.pathid = buffer:ReadString()
					end
					self.prefixid = buffer:ReadUInt(3)
					self.filename = buffer:ReadString()
					self.crc = buffer:ReadUInt(32)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_FileCRCCheck, NET_MESSAGE_BITS)
					buffer:WriteBit(self.reserved and 1 or 0)
					buffer:WriteUInt(self.gamepath, 2)
					if self.gamepath == 0 then
						buffer:WriteString(self.pathid)
					end
					buffer:WriteUInt(self.prefixid, 3)
					buffer:WriteString(self.filename)
					buffer:WriteUInt(self.crc, 32)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.reserved = nil
					self.gamepath = nil
					self.pathid = nil
					self.prefixid = nil
					self.filename = nil
					self.crc = nil
				end
			}
		},

		[clc_CmdKeyValues] = { -- 16
			__tostring = function(self)
				if not self.initialized then
					return "clc_CmdKeyValues"
				end

				return "clc_CmdKeyValues: " .. self.length
			end,
			__index = {
				GetType = function()
					return clc_CmdKeyValues
				end,
				GetName = function()
					return "clc_CmdKeyValues"
				end,
				ReadFromBuffer = function(self, buffer)
					self.length = buffer:ReadUInt(32)
					if self.length > 0 then
						self.keyvalues = buffer:ReadBytes(self.length)
					end
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_CmdKeyValues, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.length, 32)
					if self.length > 0 then
						buffer:WriteBytes(self.keyvalues)
					end
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.length = nil
					self.keyvalues = nil
				end
			}
		},

		[clc_FileMD5Check] = { -- 17
			__tostring = function(self)
				if not self.initialized then
					return "clc_FileMD5Check"
				end

				return string.format("clc_FileMD5Check: reserved = %s, gamepath = %s, pathid = %s, prefixid = %i, filename = %s, md5 = %s", self.reserved, self.gamepath, self.pathid, self.prefixid, self.filename, self.md5)
			end,
			__index = {
				GetType = function()
					return clc_FileMD5Check
				end,
				GetName = function()
					return "clc_FileMD5Check"
				end,
				ReadFromBuffer = function(self, buffer)
					self.reserved = buffer:ReadBit() == 1
					self.gamepath = buffer:ReadUInt(2)
					self.pathid = "commonpath"
					if self.gamepath == 0 then
						self.pathid = buffer:ReadString()
					end
					self.prefixid = buffer:ReadUInt(3)
					self.filename = buffer:ReadString()
					self.md5 = buffer:ReadBytes(16)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_FileMD5Check, NET_MESSAGE_BITS)
					buffer:WriteBit(self.reserved and 1 or 0)
					buffer:WriteUInt(self.gamepath, 2)
					if self.gamepath == 0 then
						buffer:WriteString(self.pathid)
					end
					buffer:WriteUInt(self.prefixid, 3)
					buffer:WriteString(self.filename)
					buffer:WriteUInt(self.md5, 16)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.reserved = nil
					self.gamepath = nil
					self.pathid = nil
					self.prefixid = nil
					self.filename = nil
					self.md5 = nil
				end
			}
		},

		[clc_GMod_ClientToServer] = { -- 18
			__tostring = function(self)
				if not self.initialized then
					return "clc_GMod_ClientToServer"
				end

				if self.msgtype == 0 then
					return string.format("clc_GMod_ClientToServer netmessage: bits = %i, msgtype = %i, id = %i/%s", self.bits, self.msgtype, self.id, util.NetworkIDToString(self.id) or "unknown message")
				elseif self.msgtype == 2 then
					return string.format("clc_GMod_ClientToServer client Lua error: %s", self.strerr)
				elseif self.msgtype == 4 then
					self.count = self.bits / 16
					if self.count > 0 then
						return string.format("clc_GMod_ClientToServer GModDataPack::SendFileToClient: bits = %i, count = %i", self.bits, self.count)
					else
						return "clc_GMod_ClientToServer GModDataPack::SendFileToClient"
					end
				end

				return "clc_GMod_ClientToServer invalid"
			end,
			__index = {
				GetType = function()
					return clc_GMod_ClientToServer
				end,
				GetName = function()
					return "clc_GMod_ClientToServer"
				end,
				ReadFromBuffer = function(self, buffer)
					local bits = buffer:ReadUInt(20)
					self.bits = bits
					self.msgtype = buffer:ReadByte()
					bits = bits - 8

					if self.msgtype == 0 then
						self.id = buffer:ReadWord()
						bits = bits - 16

						if bits > 0 then
							self.data = buffer:ReadBits(bits)
						end
					elseif self.msgtype == 2 then
						self.strerr = buffer:ReadString()
					elseif self.msgtype == 4 then
						self.count = bits / 16

						self.ids = {}
						for i = 1, self.count do
							self.ids[i] = buffer:ReadUInt(16)
						end
					end

					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(clc_GMod_ClientToServer, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.bits, 20)
					buffer:WriteByte(self.msgtype)

					if self.msgtype == 0 then
						buffer:WriteWord(self.id)

						if self.data ~= nil then
							buffer:WriteBits(self.data)
						end
					elseif self.msgtype == 2 then
						buffer:WriteString(self.strerr)
					elseif self.msgtype == 4 then
						for i = 1, self.count do
							buffer:WriteUInt(self.ids[i], 16)
						end
					end

					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.bits = nil
					self.msgtype = nil
					self.id = nil
					self.data = nil
					self.strerr = nil
					self.ids = nil
				end
			}
		},
	},

	SVC = {
		[svc_Print] = { -- 7
			__tostring = function(self)
				if not self.initialized then
					return "svc_Print"
				end

				return "svc_Print: " .. self.str
			end,
			__index = {
				GetType = function()
					return svc_Print
				end,
				GetName = function()
					return "svc_Print"
				end,
				ReadFromBuffer = function(self, buffer)
					self.str = buffer:ReadString()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_Print, NET_MESSAGE_BITS)
					buffer:WriteString(self.str)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.str = nil
				end
			}
		},

		[svc_ServerInfo] = { -- 8
			__tostring = function(self)
				if not self.initialized then
					return "svc_ServerInfo"
				end

				return string.format("svc_ServerInfo: version = %i, servercount = %i, sourcetv = %s, dedicated = %s, serverclientcrc = %i, maxclasses = %i, servermapmd5 = %s, playernum = %i, maxclients = %i, interval_per_tick = %i, platform = %s, gamedir = %s, levelname = %s, skyname = %s, hostname = %s, loadingurl = %s, gamemode = %s", self.version, self.servercount, self.sourcetv, self.dedicated, self.serverclientcrc, self.maxclasses, self.servermapmd5, self.playernum, self.maxclients, self.interval_per_tick, string.char(self.platform), self.gamedir, self.levelname, self.skyname, self.hostname, self.loadingurl, self.gamemode)
			end,
			__index = {
				GetType = function()
					return svc_ServerInfo
				end,
				GetName = function()
					return "svc_ServerInfo"
				end,
				ReadFromBuffer = function(self, buffer)
					-- Protocol version number
					self.version = buffer:ReadShort()
					-- # of servers spawned since server .exe started
					-- So that we can detect new server startup during download, etc.
					-- Map change causes new server to "spawn".
					self.servercount = buffer:ReadLong()
					-- Is SourceTV enabled?
					self.sourcetv = buffer:ReadBit() == 1
					-- 0 == listen, 1 == dedicated
					self.dedicated = buffer:ReadBit() == 1
					-- The client side DLL CRC check.
					self.serverclientcrc = buffer:ReadLong()
					-- Max amount of 'classes' (entity classes?)
					self.maxclasses = buffer:ReadWord()
					-- The MD5 of the server map must match the MD5 of the client map, else
					-- the client is probably cheating.
					self.servermapmd5 = buffer:ReadBytes(16)
					-- Amount of clients currently connected
					self.playernum = buffer:ReadByte()
					-- Max amount of clients
					self.maxclients = buffer:ReadByte()
					-- Interval between ticks
					self.interval_per_tick = buffer:ReadFloat()
					-- Server platform ('w', ...?)
					self.platform = buffer:ReadChar()
					-- Directory used by game (eg. garrysmod)
					self.gamedir = buffer:ReadString()
					-- Map being played
					self.levelname = buffer:ReadString()
					-- Skybox to use
					self.skyname = buffer:ReadString()
					-- Server name
					self.hostname = buffer:ReadString()
					-- Loading URL of the server
					self.loadingurl = buffer:ReadString()
					-- Gamemode
					self.gamemode = buffer:ReadString()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_ServerInfo, NET_MESSAGE_BITS)
					buffer:WriteShort(self.version)
					buffer:WriteLong(self.servercount)
					buffer:WriteBit(self.sourcetv and 1 or 0)
					buffer:WriteBit(self.dedicated and 1 or 0)
					buffer:WriteLong(self.serverclientcrc)
					buffer:WriteWord(self.maxclasses)
					buffer:WriteBytes(self.servermapmd5)
					buffer:WriteByte(self.playernum)
					buffer:WriteByte(self.maxclients)
					buffer:WriteFloat(self.interval_per_tick)
					buffer:WriteChar(self.platform)
					buffer:WriteString(self.gamedir)
					buffer:WriteString(self.levelname)
					buffer:WriteString(self.skyname)
					buffer:WriteString(self.hostname)
					buffer:WriteString(self.loadingurl)
					buffer:WriteString(self.gamemode)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.version = nil
					self.servercount = nil
					self.sourcetv = nil
					self.dedicated = nil
					self.serverclientcrc = nil
					self.maxclasses = nil
					self.servermapmd5 = nil
					self.playernum = nil
					self.maxclients = nil
					self.interval_per_tick = nil
					self.platform = nil
					self.gamedir = nil
					self.levelname = nil
					self.skyname = nil
					self.hostname = nil
					self.loadingurl = nil
					self.gamemode = nil
				end
			}
		},

		[svc_SendTable] = { -- 9
			__tostring = function(self)
				if not self.initialized then
					return "svc_SendTable"
				end

				return string.format("svc_SendTable: encoded = %s, bits = %i", self.encoded, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_SendTable
				end,
				GetName = function()
					return "svc_SendTable"
				end,
				ReadFromBuffer = function(self, buffer)
					self.encoded = buffer:ReadBit() == 1
					self.bits = buffer:ReadWord()
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_SendTable, NET_MESSAGE_BITS)
					buffer:WriteBit(self.encoded and 1 or 0)
					buffer:WriteWord(self.bits)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.encoded = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_ClassInfo] = { -- 10
			__tostring = function(self)
				if not self.initialized then
					return "svc_ClassInfo"
				end

				return string.format("svc_ClassInfo: numclasses = %i, useclientclasses = %s", self.numclasses, self.useclientclasses)
			end,
			__index = {
				GetType = function()
					return svc_ClassInfo
				end,
				GetName = function()
					return "svc_ClassInfo"
				end,
				ReadFromBuffer = function(self, buffer)
					self.numclasses = buffer:ReadWord()
					self.useclientclasses = buffer:ReadBit() == 1
					if not self.useclientclasses then
						self.classes = {}
						local size = log2(self.numclasses) + 1
						for i = 1, self.numclasses do
							self.classes[i] = {}
							self.classes[i].classid = buffer:ReadUInt(size)
							self.classes[i].classname = buffer:ReadString()
							self.classes[i].dtname = buffer:ReadString()
						end
					end
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_ClassInfo, NET_MESSAGE_BITS)
					buffer:WriteWord(self.numclasses)
					buffer:WriteBit(self.useclientclasses and 1 or 0)
					if not self.useclientclasses then
						local size = log2(self.numclasses) + 1
						for i = 1, self.numclasses do
							buffer:WriteUInt(self.classes[i].classid, size)
							buffer:WriteString(self.classes[i].classname)
							buffer:WriteString(self.classes[i].dtname)
						end
					end

					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.numclasses = nil
					self.useclientclasses = nil
					self.classes = nil
				end
			}
		},

		[svc_SetPause] = { -- 11
			__tostring = function(self)
				if not self.initialized then
					return "svc_SetPause"
				end

				return "svc_SetPause: " .. tostring(self.paused)
			end,
			__index = {
				GetType = function()
					return svc_SetPause
				end,
				GetName = function()
					return "svc_SetPause"
				end,
				ReadFromBuffer = function(self, buffer)
					self.paused = buffer:ReadBit() == 1
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_SetPause, NET_MESSAGE_BITS)
					buffer:WriteBit(self.paused and 1 or 0)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.paused = nil
				end
			}
		},

		[svc_CreateStringTable] = { -- 12
			__tostring = function(self)
				if not self.initialized then
					return "svc_CreateStringTable"
				end

				return string.format("svc_CreateStringTable: tablename = %s, maxentries = %d, bits = %d, userdata = %s, userdatasize = %d, userdatabits = %d, compressed = %s", self.tablename, self.maxentries, self.bits, self.userdata, self.userdatasize, self.userdatabits, self.compressed)
			end,
			__index = {
				GetType = function()
					return svc_CreateStringTable
				end,
				GetName = function()
					return "svc_CreateStringTable"
				end,
				ReadFromBuffer = function(self, buffer)
					self.tablename = buffer:ReadString()
					self.maxentries = buffer:ReadWord()
					self.entries = buffer:ReadUInt(log2(self.maxentries) + 1)
					self.bits = buffer:ReadVarInt32()
					self.userdata = buffer:ReadBit() == 1
					self.userdatasize = self.userdata and buffer:ReadUInt(12) or 0
					self.userdatabits = self.userdata and buffer:ReadUInt(4) or 0
					self.compressed = buffer:ReadBit() == 1
					self.data = self.bits > 0 and buffer:ReadBits(self.bits) or nil
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_CreateStringTable, NET_MESSAGE_BITS)
					buffer:WriteString(self.tablename)
					buffer:WriteWord(self.maxentries)
					buffer:WriteUInt(self.entries, log2(self.maxentries) + 1)
					buffer:WriteVarInt32(self.bits)
					buffer:WriteBit(self.userdata and 1 or 0)
					if self.userdata then
						buffer:WriteUInt(self.userdatasize, 12)
						buffer:WriteUInt(self.userdatabits, 4)
					end
					buffer:WriteBit(self.compressed and 1 or 0)
					if self.data ~= nil then
						buffer:WriteBits(self.data)
					end
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.tablename = nil
					self.maxentries = nil
					self.entries = nil
					self.bits = nil
					self.userdata = nil
					self.userdatasize = nil
					self.userdatabits = nil
					self.compressed = nil
					self.data = nil
				end
			}
		},

		[svc_UpdateStringTable] = { -- 13
			__tostring = function(self)
				if not self.initialized then
					return "svc_UpdateStringTable"
				end

				return string.format("svc_UpdateStringTable: tableid = %i, changed = %i, bits = %i", self.tableid, self.changed, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_UpdateStringTable
				end,
				GetName = function()
					return "svc_UpdateStringTable"
				end,
				ReadFromBuffer = function(self, buffer)
					self.tableid = buffer:ReadUInt(MAX_TABLES_BITS)
					self.changed = buffer:ReadBit() == 1 and buffer:ReadUInt(16) or 1
					self.bits = buffer:ReadUInt(20)
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_UpdateStringTable, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.tableid, MAX_TABLES_BITS)
					buffer:WriteBit(self.changed == 1 and 0 or 1)
					if self.changed ~= 1 then
						buffer:WriteUInt(self.changed, 16)
					end
					buffer:WriteUInt(self.bits, 20)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.tableid = nil
					self.changed = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_VoiceInit] = { -- 14
			__tostring = function(self)
				if not self.initialized then
					return "svc_VoiceInit"
				end

				return string.format("svc_VoiceInit: codec = %s, quality = %i", self.codec, self.quality)
			end,
			__index = {
				GetType = function()
					return svc_VoiceInit
				end,
				GetName = function()
					return "svc_VoiceInit"
				end,
				ReadFromBuffer = function(self, buffer)
					self.codec = buffer:ReadString()
					self.quality = buffer:ReadByte()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_VoiceInit, NET_MESSAGE_BITS)
					buffer:WriteString(self.codec)
					buffer:WriteByte(self.quality)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.codec = nil
					self.quality = nil
				end
			}
		},

		[svc_VoiceData] = { -- 15
			__tostring = function(self)
				if not self.initialized then
					return "svc_VoiceData"
				end

				return string.format("svc_VoiceData: client = %i, proximity = %i, bits = %i", self.client, self.proximity, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_VoiceData
				end,
				GetName = function()
					return "svc_VoiceData"
				end,
				ReadFromBuffer = function(self, buffer)
					self.client = buffer:ReadByte()
					self.proximity = buffer:ReadByte()
					self.bits = buffer:ReadWord()
					if self.bits > 0 then
						self.voicedata = buffer:ReadBits(self.bits)
					end
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_VoiceData, NET_MESSAGE_BITS)
					buffer:WriteByte(self.client)
					buffer:WriteByte(self.proximity)
					buffer:WriteWord(self.bits)
					if self.voicedata ~= nil then
						buffer:WriteBits(self.voicedata)
					end
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.client = nil
					self.proximity = nil
					self.bits = nil
					self.voicedata = nil
				end
			}
		},

		[svc_Sounds] = { -- 17
			__tostring = function(self)
				if not self.initialized then
					return "svc_Sounds"
				end

				return string.format("svc_Sounds: reliable = %s, num = %i, bits = %i\n", self.reliable, self.num, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_Sounds
				end,
				GetName = function()
					return "svc_Sounds"
				end,
				ReadFromBuffer = function(self, buffer)
					self.reliable = buffer:ReadBit() == 1
					self.num = self.reliable and 1 or buffer:ReadUInt(8)
					self.bits = self.reliable and buffer:ReadUInt(8) or buffer:ReadUInt(16)
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_Sounds, NET_MESSAGE_BITS)
					buffer:WriteBit(self.reliable and 1 or 0)
					if not self.reliable then
						buffer:WriteUInt(self.num, 8)
						buffer:WriteUInt(self.bits, 16)
					else
						buffer:WriteUInt(self.bits, 8)
					end
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.reliable = nil
					self.num = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_SetView] = { -- 18
			__tostring = function(self)
				if not self.initialized then
					return "svc_SetView"
				end

				return "svc_SetView: viewent = " .. self.viewent
			end,
			__index = {
				GetType = function()
					return svc_SetView
				end,
				GetName = function()
					return "svc_SetView"
				end,
				ReadFromBuffer = function(self, buffer)
					self.viewent = buffer:ReadUInt(MAX_EDICT_BITS)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_SetView, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.viewent, MAX_EDICT_BITS)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.viewent = nil
				end
			}
		},

		[svc_FixAngle] = { -- 19
			__tostring = function(self)
				if not self.initialized then
					return "svc_FixAngle"
				end

				return string.format("svc_FixAngle: relative = %s, x = %i, y = %i, z = %i", self.relative, self.x, self.y, self.z)
			end,
			__index = {
				GetType = function()
					return svc_FixAngle
				end,
				GetName = function()
					return "svc_FixAngle"
				end,
				ReadFromBuffer = function(self, buffer)
					self.relative = buffer:ReadBit() == 1
					self.x = buffer:ReadBitAngle(16)
					self.y = buffer:ReadBitAngle(16)
					self.z = buffer:ReadBitAngle(16)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_FixAngle, NET_MESSAGE_BITS)
					buffer:WriteBit(self.relative and 1 or 0)
					buffer:WriteBitAngle(self.x, 16)
					buffer:WriteBitAngle(self.y, 16)
					buffer:WriteBitAngle(self.z, 16)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.relative = nil
					self.x = nil
					self.y = nil
					self.z = nil
				end
			}
		},

		[svc_CrosshairAngle] = { -- 20
			__tostring = function(self)
				if not self.initialized then
					return "svc_CrosshairAngle"
				end

				return string.format("svc_CrosshairAngle: p = %i, y = %i, r = %i", self.p, self.y, self.r)
			end,
			__index = {
				GetType = function()
					return svc_CrosshairAngle
				end,
				GetName = function()
					return "svc_CrosshairAngle"
				end,
				ReadFromBuffer = function(self, buffer)
					self.p = buffer:ReadBitAngle(16)
					self.y = buffer:ReadBitAngle(16)
					self.r = buffer:ReadBitAngle(16)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_CrosshairAngle, NET_MESSAGE_BITS)
					buffer:WriteBitAngle(self.p, 16)
					buffer:WriteBitAngle(self.y, 16)
					buffer:WriteBitAngle(self.r, 16)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.p = nil
					self.y = nil
					self.r = nil
				end
			}
		},

		[svc_BSPDecal] = { -- 21
			__tostring = function(self)
				if not self.initialized then
					return "svc_BSPDecal"
				end

				return string.format("svc_BSPDecal: pos = %s, texture = %d, useentity = %s, ent = %d, modulation = %d, lowpriority = %s", tostring(self.pos), self.texture, self.useentity, self.ent, self.modulation, self.lowpriority)
			end,
			__index = {
				GetType = function()
					return svc_BSPDecal
				end,
				GetName = function()
					return "svc_BSPDecal"
				end,
				ReadFromBuffer = function(self, buffer)
					self.pos = buffer:ReadVector()
					self.texture = buffer:ReadUInt(9)
					self.useentity = buffer:ReadBit() == 1
					if self.useentity then
						self.ent = buffer:ReadUInt(MAX_EDICT_BITS)
						self.modulation = buffer:ReadUInt(12)
					else
						self.ent = 0
						self.modulation = 0
					end
					self.lowpriority = buffer:ReadBit() == 1
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_BSPDecal, NET_MESSAGE_BITS)
					buffer:WriteVector(self.pos)
					buffer:WriteUInt(self.texture, 9)
					buffer:WriteBit(self.useentity and 1 or 0)
					if self.useentity then
						buffer:WriteUInt(self.ent, MAX_EDICT_BITS)
						buffer:WriteUInt(self.modulation, 12)
					end
					buffer:WriteBit(self.lowpriority and 1 or 0)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.pos = nil
					self.texture = nil
					self.useentity = nil
					self.ent = nil
					self.modulation = nil
					self.lowpriority = nil
				end
			}
		},

		[svc_UserMessage] = { -- 23
			__tostring = function(self)
				if not self.initialized then
					return "svc_UserMessage"
				end

				return string.format("svc_UserMessage: msgtype = %i, bits = %i", self.msgtype, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_UserMessage
				end,
				GetName = function()
					return "svc_UserMessage"
				end,
				ReadFromBuffer = function(self, buffer)
					self.msgtype = buffer:ReadByte()
					self.bits = buffer:ReadUInt(MAX_USERMESSAGE_BITS)
					if self.bits > 0 then
						self.data = buffer:ReadBits(self.bits)
					end
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_UserMessage, NET_MESSAGE_BITS)
					buffer:WriteByte(self.msgtype)
					buffer:WriteUInt(self.bits, MAX_USERMESSAGE_BITS)
					if self.data ~= nil then
						buffer:WriteBits(self.data)
					end
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.msgtype = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_EntityMessage] = { -- 24
			__tostring = function(self)
				if not self.initialized then
					return "svc_EntityMessage"
				end

				return string.format("svc_EntityMessage: entity = %i, class = %i, bits = %i", self.entity, self.class, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_EntityMessage
				end,
				GetName = function()
					return "svc_EntityMessage"
				end,
				ReadFromBuffer = function(self, buffer)
					self.entity = buffer:ReadUInt(MAX_EDICT_BITS)
					self.class = buffer:ReadUInt(MAX_SERVER_CLASS_BITS)
					self.bits = buffer:ReadUInt(MAX_ENTITYMESSAGE_BITS)
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_EntityMessage, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.entity, MAX_EDICT_BITS)
					buffer:WriteUInt(self.class, MAX_SERVER_CLASS_BITS)
					buffer:WriteUInt(self.bits, MAX_ENTITYMESSAGE_BITS)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.entity = nil
					self.class = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_GameEvent] = { -- 25
			__tostring = function(self)
				if not self.initialized then
					return "svc_GameEvent"
				end

				return "svc_GameEvent: bits = " .. self.bits
			end,
			__index = {
				GetType = function()
					return svc_GameEvent
				end,
				GetName = function()
					return "svc_GameEvent"
				end,
				ReadFromBuffer = function(self, buffer)
					self.bits = buffer:ReadUInt(11)
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_GameEvent, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.bits, 11)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_PacketEntities] = { -- 26
			__tostring = function(self)
				if not self.initialized then
					return "svc_PacketEntities"
				end

				return string.format("svc_PacketEntities: max = %i, isdelta = %s, delta = %i, baseline = %s, changed = %i, bits = %i, updatebaseline = %s", self.max, self.isdelta, self.delta, self.baseline, self.changed, self.bits, self.updatebaseline)
			end,
			__index = {
				GetType = function()
					return svc_PacketEntities
				end,
				GetName = function()
					return "svc_PacketEntities"
				end,
				ReadFromBuffer = function(self, buffer)
					self.max = buffer:ReadUInt(MAX_EDICT_BITS)
					self.isdelta = buffer:ReadBit() == 1
					self.delta = self.isdelta and buffer:ReadLong() or -1
					self.baseline = buffer:ReadBit() == 1
					self.changed = buffer:ReadUInt(MAX_EDICT_BITS)
					self.bits = buffer:ReadUInt(24)
					self.updatebaseline = buffer:ReadBit() == 1
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_PacketEntities, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.max, MAX_EDICT_BITS)
					buffer:WriteBit(self.isdelta and 1 or 0)
					if self.isdelta then
						buffer:WriteLong(self.delta)
					end
					buffer:WriteBit(self.baseline and 1 or 0)
					buffer:WriteUInt(self.changed, MAX_EDICT_BITS)
					buffer:WriteUInt(self.bits, 24)
					buffer:WriteBit(self.updatebaseline and 1 or 0)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.max = nil
					self.isdelta = nil
					self.delta = nil
					self.baseline = nil
					self.changed = nil
					self.bits = nil
					self.updatebaseline = nil
					self.data = nil
				end
			}
		},

		[svc_TempEntities] = { -- 27
			__tostring = function(self)
				if not self.initialized then
					return "svc_TempEntities"
				end

				return string.format("svc_TempEntities: num = %i, bits = %i\n", self.num, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_TempEntities
				end,
				GetName = function()
					return "svc_TempEntities"
				end,
				ReadFromBuffer = function(self, buffer)
					self.num = buffer:ReadUInt(8)
					self.bits = buffer:ReadVarInt32()
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_TempEntities, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.num, 8)
					buffer:WriteVarInt32(self.bits)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.num = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_Prefetch] = { -- 28
			__tostring = function(self)
				if not self.initialized then
					return "svc_Prefetch"
				end

				return "svc_Prefetch: index = " .. self.index
			end,
			__index = {
				GetType = function()
					return svc_Prefetch
				end,
				GetName = function()
					return "svc_Prefetch"
				end,
				ReadFromBuffer = function(self, buffer)
					self.index = buffer:ReadUInt(14)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_Prefetch, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.index, 14)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.index = nil
				end
			}
		},

		[svc_Menu] = { -- 29
			__tostring = function(self)
				if not self.initialized then
					return "svc_Menu"
				end

				return string.format("svc_Menu: menutype = %i, bytes = %i", self.menutype, self.bytes)
			end,
			__index = {
				GetType = function()
					return svc_Menu
				end,
				GetName = function()
					return "svc_Menu"
				end,
				ReadFromBuffer = function(self, buffer)
					self.menutype = buffer:ReadWord()
					self.bytes = buffer:ReadWord()
					self.data = buffer:ReadBytes(self.bytes)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_Menu, NET_MESSAGE_BITS)
					buffer:WriteWord(self.menutype)
					buffer:WriteWord(self.bytes)
					buffer:WriteBytes(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.menutype = nil
					self.bytes = nil
					self.data = nil
				end
			}
		},

		[svc_GameEventList] = { -- 30
			__tostring = function(self)
				if not self.initialized then
					return "svc_GameEventList"
				end

				return string.format("svc_GameEventList: num = %i, bits = %i", self.num, self.bits)
			end,
			__index = {
				GetType = function()
					return svc_GameEventList
				end,
				GetName = function()
					return "svc_GameEventList"
				end,
				ReadFromBuffer = function(self, buffer)
					self.num = buffer:ReadUInt(9)
					self.bits = buffer:ReadUInt(20)
					self.data = buffer:ReadBits(self.bits)
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_GameEventList, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.num, 9)
					buffer:WriteUInt(self.bits, 20)
					buffer:WriteBits(self.data)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.num = nil
					self.bits = nil
					self.data = nil
				end
			}
		},

		[svc_GetCvarValue] = { -- 31
			__tostring = function(self)
				if not self.initialized then
					return "svc_GetCvarValue"
				end

				return "svc_GetCvarValue: cvarname = " .. self.cvarname
			end,
			__index = {
				GetType = function()
					return svc_GetCvarValue
				end,
				GetName = function()
					return "svc_GetCvarValue"
				end,
				ReadFromBuffer = function(self, buffer)
					self.cookie = buffer:ReadInt(32)
					self.cvarname = buffer:ReadString()
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_GetCvarValue, NET_MESSAGE_BITS)
					buffer:WriteInt(self.cookie, 32)
					buffer:WriteString(self.cvarname)
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.cookie = nil
					self.cvarname = nil
				end
			}
		},

		[svc_CmdKeyValues] = { -- 32
			__tostring = function(self)
				if not self.initialized then
					return "svc_CmdKeyValues"
				end

				return "svc_CmdKeyValues: length = " .. self.length
			end,
			__index = {
				GetType = function()
					return svc_CmdKeyValues
				end,
				GetName = function()
					return "svc_CmdKeyValues"
				end,
				ReadFromBuffer = function(self, buffer)
					self.length = buffer:ReadUInt(32)
					if self.length > 0 then
						self.keyvalues = buffer:ReadBits(self.length * 8)
					end
					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_CmdKeyValues, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.length, 32)
					if self.keyvalues then
						buffer:WriteBits(self.keyvalues)
					end
					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.length = nil
					self.keyvalues = nil
				end
			}
		},

		[svc_GMod_ServerToClient] = { -- 33
			__tostring = function(self)
				if not self.initialized then
					return "svc_GMod_ServerToClient"
				end

				if self.msgtype == 0 then
					return string.format("svc_GMod_ServerToClient netmessage: bits = %i, id = %i/%s", self.bits, self.id, util.NetworkIDToString(self.id) or "unknown message")
				elseif self.msgtype == 1 then
					return string.format("svc_GMod_ServerToClient auto-refresh: length = %i, path = %s", self.length, self.path)
				elseif self.msgtype == 3 then
					return "svc_GMod_ServerToClient GModDataPack::RequestFiles: bits = " .. self.bits
				elseif self.msgtype == 4 then
					return "svc_GMod_ServerToClient GModDataPack::UpdateFile: length = " .. self.length
				end

				return "svc_GMod_ServerToClient invalid"
			end,
			__index = {
				GetType = function()
					return svc_GMod_ServerToClient
				end,
				GetName = function()
					return "svc_GMod_ServerToClient"
				end,
				ReadFromBuffer = function(self, buffer)
					local bits = buffer:ReadUInt(20)
					self.bits = bits
					self.msgtype = buffer:ReadByte()
					bits = bits - 8

					if self.msgtype == 0 then
						self.id = buffer:ReadWord()
						self.length = bits - 16

						if self.length > 0 then
							self.data = buffer:ReadBits(self.length)
						end
					elseif self.msgtype == 1 then
						self.path = buffer:ReadString()
						self.length = buffer:ReadUInt(32)
						self.data = buffer:ReadBytes(self.length)
					elseif self.msgtype == 3 then
						self.length = bits

						if self.length > 0 then
							self.data = buffer:ReadBits(self.length)
						end
					elseif self.msgtype == 4 then
						self.id = buffer:ReadUInt(16)
						self.length = bits - 16

						if self.length > 0 then
							self.data = buffer:ReadBits(self.length)
						end
					end

					self.initialized = true
				end,
				WriteToBuffer = function(self, buffer)
					if not self.initialized then
						return false
					end

					buffer:WriteUInt(svc_GMod_ServerToClient, NET_MESSAGE_BITS)
					buffer:WriteUInt(self.bits, 20)
					buffer:WriteByte(self.msgtype)

					if self.msgtype == 0 then
						buffer:WriteWord(self.id)

						if self.data ~= nil then
							buffer:WriteBits(self.data)
						end
					elseif self.msgtype == 1 then
						buffer:WriteString(self.path)
						buffer:WriteUInt(self.length)
						buffer:WriteBytes(self.data)
					elseif self.msgtype == 3 then
						if self.data ~= nil then
							buffer:WriteBits(self.data)
						end
					elseif self.msgtype == 4 then
						buffer:WriteUInt(self.id, 16)

						if self.data ~= nil then
							buffer:WriteBits(self.data)
						end
					end

					return true
				end,
				Reset = function(self)
					self.initialized = nil
					self.bits = nil
					self.msgtype = nil
					self.id = nil
					self.data = nil
					self.path = nil
					self.length = nil
				end
			}
		}
	}
}

local NET_MESSAGES_CONSTRUCTORS = {
	NET = {},
	CLC = {},
	SVC = {}
}

for net_messages_prefix, net_messages in pairs(NET_MESSAGES) do
	for msgtype, metatable in pairs(net_messages) do
		NET_MESSAGES_CONSTRUCTORS[net_messages_prefix][msgtype] = function()
			return setmetatable({}, metatable)
		end
	end
end

local NET_MESSAGES_NATIVE_CONSTRUCTORS = {
	NET = {
		[net_NOP] = NET_MESSAGES_CONSTRUCTORS.NET[net_NOP],
		[net_Disconnect] = NET_Disconnect,
		[net_File] = NET_File,
		[net_Tick] = NET_Tick,
		[net_StringCmd] = NET_StringCmd,
		[net_SetConVar] = NET_SetConVar,
		[net_SignonState] = NET_SignonState
	},

	CLC = {
		[clc_ClientInfo] = CLC_ClientInfo,
		[clc_Move] = CLC_Move,
		[clc_VoiceData] = CLC_VoiceData,
		[clc_BaselineAck] = CLC_BaselineAck,
		[clc_ListenEvents] = CLC_ListenEvents,
		[clc_RespondCvarValue] = CLC_RespondCvarValue,
		[clc_FileCRCCheck] = CLC_FileCRCCheck,
		[clc_CmdKeyValues] = CLC_CmdKeyValues,
		[clc_FileMD5Check] = CLC_FileMD5Check,
		[clc_GMod_ClientToServer] = CLC_GMod_ClientToServer
	},

	SVC = {
		[svc_Print] = SVC_Print,
		[svc_ServerInfo] = SVC_ServerInfo,
		[svc_SendTable] = SVC_SendTable,
		[svc_ClassInfo] = SVC_ClassInfo,
		[svc_SetPause] = SVC_SetPause,
		[svc_CreateStringTable] = SVC_CreateStringTable,
		[svc_UpdateStringTable] = SVC_UpdateStringTable,
		[svc_VoiceInit] = SVC_VoiceInit,
		[svc_VoiceData] = SVC_VoiceData,
		[svc_Sounds] = SVC_Sounds,
		[svc_SetView] = SVC_SetView,
		[svc_FixAngle] = SVC_FixAngle,
		[svc_CrosshairAngle] = SVC_CrosshairAngle,
		[svc_BSPDecal] = SVC_BSPDecal,
		[svc_UserMessage] = SVC_UserMessage,
		[svc_EntityMessage] = SVC_EntityMessage,
		[svc_GameEvent] = SVC_GameEvent,
		[svc_PacketEntities] = SVC_PacketEntities,
		[svc_TempEntities] = SVC_TempEntities,
		[svc_Prefetch] = SVC_Prefetch,
		[svc_Menu] = SVC_Menu,
		[svc_GameEventList] = SVC_GameEventList,
		[svc_GetCvarValue] = SVC_GetCvarValue,
		[svc_CmdKeyValues] = SVC_CmdKeyValues,
		[svc_GMod_ServerToClient] = SVC_GMod_ServerToClient
	}
}

function NetMessage(netchan, msgtype, server)
	local constructor = NET_MESSAGES_CONSTRUCTORS.NET[msgtype]
	if constructor == nil then
		if server then
			constructor = NET_MESSAGES_CONSTRUCTORS.SVC[msgtype]
		else
			constructor = NET_MESSAGES_CONSTRUCTORS.CLC[msgtype]
		end

		if constructor == nil then
			return
		end
	end

	return constructor(netchan)
end
