local cfg = {
    AddrBytes = 4,
    DataBytes = 2,
    LenBytes = 1,
    CrcBytes = 2,
    LocalPort = 1234,
    RemotePort = 4321,
}

local rap_protocol = Proto("RAP", "Register Access Protocol")

local function makeCfgField(cfg_val)
    if (cfg_val == 1) then
        return ProtoField.uint8
    elseif (cfg_val == 2) then
        return ProtoField.uint16
    elseif (cfg_val == 3 or cfg_val == 4) then
        return ProtoField.uint32
    elseif (cfg_val > 4 and cfg_val < 9) then
        return ProtoField.uint64
    end
end

local fields = {
    txn_id = ProtoField.uint8("rap.txn_id", "Transaction ID", base.HEX),
    msg_type = ProtoField.uint8("rap.msg_type", "Message Type", base.HEX),
    payload = ProtoField.ubytes("rap.payload", "Message Payload", base.SPACE),
    addr = makeCfgField(cfg.AddrBytes)("rap.addr", "Address", base.HEX),
    data = makeCfgField(cfg.DataBytes)("rap.data", "Data", base.HEX),
    mask = makeCfgField(cfg.DataBytes)("rap.mask", "Mask", base.HEX),
    status = makeCfgField(cfg.DataBytes)("rap.status", "Status", base.HEX),
    increment = makeCfgField(cfg.DataBytes)("rap.incr", "Increment", base.DEC),
    count = makeCfgField(cfg.DataBytes)("rap.count", "Count", base.DEC),
    length = makeCfgField(cfg.LenBytes)("rap.length", "Length", base.DEC),
    crc = makeCfgField(cfg.CrcBytes)("rap.crc", "CRC", base.HEX),
}
local pfields = {}
for _,v in pairs(fields) do
    pfields[#pfields+1] = v
end
rap_protocol.fields = pfields

local MessageType_FromValue = {
    [0x01] = "CmdSingleRead",
    [0x80] = "AckSingleRead",
    [0xC1] = "NakSingleRead",
    [0x10] = "CmdSingleWrite",
    [0x51] = "CmdSingleWritePosted",
    [0x91] = "AckSingleWrite",
    [0xD0] = "NakSingleWrite",
    [0x04] = "CmdSeqRead",
    [0x85] = "AckSeqRead",
    [0xC4] = "NakSeqRead",
    [0x15] = "CmdSeqWrite",
    [0x54] = "CmdSeqWritePosted",
    [0x94] = "AckSeqWrite",
    [0xD5] = "NakSeqWrite",
    [0x08] = "CmdCompRead",
    [0x89] = "AckCompRead",
    [0xC8] = "NakCompRead",
    [0x19] = "CmdCompWrite",
    [0x58] = "CmdCompWritePosted",
    [0x98] = "AckCompWrite",
    [0xD9] = "NakCompWrite",
    [0x20] = "CmdSingleRmw",
    [0x61] = "CmdSingleRmwPosted",
    [0xA1] = "AckSingleRmw",
    [0xE0] = "NakSingleRmw",
    [0xB0] = "AckSingleInterrupt",
}
local function msgTypeByteToString(v)
    local str = MessageType_FromValue[v]
    if str ~= nil then
        return str
    else
        return "<unknown>"
    end
end

local MessageType = {
    eCmdSingleRead = 0x01,
    eAckSingleRead = 0x80,
    eNakSingleRead = 0xC1,
    eCmdSingleWrite = 0x10,
    eCmdSingleWritePosted = 0x51,
    eAckSingleWrite = 0x91,
    eNakSingleWrite = 0xD0,
    eCmdSeqRead = 0x04,
    eAckSeqRead = 0x85,
    eNakSeqRead = 0xC4,
    eCmdSeqWrite = 0x15,
    eCmdSeqWritePosted = 0x54,
    eAckSeqWrite = 0x94,
    eNakSeqWrite = 0xD5,
    eCmdCompRead = 0x08,
    eAckCompRead = 0x89,
    eNakCompRead = 0xC8,
    eCmdCompWrite = 0x19,
    eCmdCompWritePosted = 0x58,
    eAckCompWrite = 0x98,
    eNakCompWrite = 0xD9,
    eCmdSingleRmw = 0x20,
    eCmdSingleRmwPosted = 0x61,
    eAckSingleRmw = 0xA1,
    eNakSingleRmw = 0xE0,
    eAckSingleInterrupt = 0xB0,
}

local function incrToType(incr)
    if (incr == 0) then
        return "FIFO"
    elseif (incr == cfg.DataBytes) then
        return "SEQ"
    else
        return "INCR"
    end
end

function rap_protocol.dissector(buffer, pinfo, tree)
    length = buffer:len()
    if length == 0 then return end

    pinfo.cols.protocol = rap_protocol.name

    local subtree = tree:add(rap_protocol, buffer(), "RAP Data")

    print(type(fields.txn_id), fields.txn_id)
    subtree:add_le(fields.txn_id, buffer(0, 1))
    local message_type = buffer:bytes(1,1):get_index(0)
    subtree:add_le(fields.msg_type, buffer(1, 1)):append_text(" (" .. msgTypeByteToString(message_type) .. ")")
    local payload_length = length - 2 - cfg.CrcBytes
    local payload_buffer = buffer(2, payload_length)
    local payload_subtree = subtree:add(rap_protocol, payload_buffer, "Message Payload"):append_text(" (" .. (payload_length) .. " Bytes)")
    subtree:add_le(fields.crc, buffer(length - cfg.CrcBytes, cfg.CrcBytes))

    if (message_type == MessageType.eCmdSingleRead) then
        payload_subtree:add_le(fields.addr, payload_buffer(0, cfg.AddrBytes))
    elseif (message_type == MessageType.eAckSingleRead) then
        payload_subtree:add_le(fields.data, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eNakSingleRead) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eCmdSingleWrite or message_type == MessageType.eCmdSingleWritePosted) then
        payload_subtree:add_le(fields.addr, payload_buffer(0, cfg.AddrBytes))
        payload_subtree:add_le(fields.data, payload_buffer(cfg.AddrBytes, cfg.DataBytes))
    elseif (message_type == MessageType.eAckSingleWrite) then
        -- No Fields
    elseif (message_type == MessageType.eNakSingleWrite) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eCmdSeqRead) then
        payload_subtree:add_le(fields.addr, payload_buffer(0, cfg.AddrBytes))
        local incr_field,incr = payload_subtree:add_packet_field(fields.increment, payload_buffer(cfg.AddrBytes, cfg.LenBytes), ENC_LITTLE_ENDIAN)
        incr_field:append_text(" (" .. incrToType(incr) .. ")")
        local _,count = payload_subtree:add_packet_field(fields.count, payload_buffer(cfg.AddrBytes + cfg.LenBytes, cfg.LenBytes), ENC_LITTLE_ENDIAN)
    elseif (message_type == MessageType.eAckSeqRead) then
        local _,count = payload_subtree:add_packet_field(fields.count, payload_buffer(0, cfg.LenBytes), ENC_LITTLE_ENDIAN)
        local fi = cfg.LenBytes
        for i=0,count-1,1 do
            payload_subtree:add_le(fields.data, payload_buffer(fi + (i*cfg.DataBytes), cfg.DataBytes))
        end
    elseif (message_type == MessageType.eNakSeqRead) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eCmdSeqWrite or message_type == MessageType.eCmdSeqWritePosted) then
        payload_subtree:add_le(fields.addr, payload_buffer(0, cfg.AddrBytes))
        local incr_field,incr = payload_subtree:add_packet_field(fields.increment, payload_buffer(cfg.AddrBytes, cfg.LenBytes), ENC_LITTLE_ENDIAN)
        incr_field:append_text(" (" .. incrToType(incr) .. ")")
        local _,count = payload_subtree:add_packet_field(fields.count, payload_buffer(cfg.AddrBytes + cfg.LenBytes, cfg.LenBytes), ENC_LITTLE_ENDIAN)
        local fi = cfg.AddrBytes + cfg.LenBytes + cfg.LenBytes
        for i=0,count-1,1 do
            payload_subtree:add_le(fields.data, payload_buffer(fi + (i*cfg.DataBytes), cfg.DataBytes))
        end
    elseif (message_type == MessageType.eAckSeqWrite) then
        -- No Fields
    elseif (message_type == MessageType.eNakSeqWrite) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eCmdCompRead) then
        local _,count = payload_subtree:add_packet_field(fields.count, payload_buffer(0, cfg.LenBytes), ENC_LITTLE_ENDIAN)
        local fi = cfg.LenBytes
        for i=0,count-1,1 do
            payload_subtree:add_le(fields.addr, payload_buffer(fi + (i*cfg.AddrBytes), cfg.AddrBytes))
        end
    elseif (message_type == MessageType.eAckCompRead) then
        local _,count = payload_subtree:add_packet_field(fields.count, payload_buffer(0, cfg.LenBytes), ENC_LITTLE_ENDIAN)
        local fi = cfg.LenBytes
        for i=0,count-1,1 do
            payload_subtree:add_le(fields.data, payload_buffer(fi + (i*cfg.DataBytes), cfg.DataBytes))
        end
    elseif (message_type == MessageType.eNakCompRead) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eCmdCompWrite or message_type == MessageType.eCmdCompWritePosted) then
        local _,count = payload_subtree:add_packet_field(fields.count, payload_buffer(0, cfg.LenBytes), ENC_LITTLE_ENDIAN)
        local fi = cfg.LenBytes
        for i=0,count-1,1 do
            local adi = fi + (i*(cfg.AddrBytes+cfg.DataBytes))
            payload_subtree:add_le(fields.addr, payload_buffer(adi, cfg.AddrBytes))
            payload_subtree:add_le(fields.data, payload_buffer(adi + cfg.AddrBytes, cfg.DataBytes))
        end
    elseif (message_type == MessageType.eAckCompWrite) then
        -- No Fields
    elseif (message_type == MessageType.eNakCompWrite) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eCmdSingleRmw or message_type == MessageType.eCmdSingleRmwPosted) then
        payload_subtree:add_le(fields.addr, payload_buffer(0, cfg.AddrBytes))
        payload_subtree:add_le(fields.data, payload_buffer(cfg.AddrBytes, cfg.DataBytes))
        payload_subtree:add_le(fields.mask, payload_buffer(cfg.AddrBytes+cfg.DataBytes, cfg.DataBytes))
    elseif (message_type == MessageType.eAckSingleRmw) then
        -- No Fields
    elseif (message_type == MessageType.eNakSingleRmw) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    elseif (message_type == MessageType.eAckSingleInterrupt) then
        payload_subtree:add_le(fields.status, payload_buffer(0, cfg.DataBytes))
    else
        payload_subtree:add("ERROR")
    end
end

local udp_port = DissectorTable.get("udp.port")
udp_port:add(cfg.LocalPort, rap_protocol)
udp_port:add(cfg.RemotePort, rap_protocol)
