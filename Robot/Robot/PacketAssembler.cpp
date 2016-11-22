#include "PacketAssembler.hpp"

using dl::robot::PacketAssembler;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PacketAssembler::AddBytes(std::string&& Bytes)
{
  mBytes += std::move(Bytes);

  AssemblePacket();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PacketAssembler::AssemblePacket()
{
  if (mBytes.size() < mHeaderSize)
  {
    return;
  }

  auto Header = dl::robot::PacketDecoder::DecodeHeader(mBytes);

  auto PacketSize = mHeaderSize + Header.mPayloadSize;

  if (mBytes.size() < PacketSize)
  {
    return;
  }

  mPacketAssembledSignal(std::experimental::string_view(mBytes.c_str(), PacketSize));

  mBytes = mBytes.substr(PacketSize);
}
