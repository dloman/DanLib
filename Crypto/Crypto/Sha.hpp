#pragma once
#include <array>
#include <cstring>
#include <string>
#include <climits>
#include <utility>

namespace dl::crypto
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  namespace err
  {
    namespace
    {
      extern const char* sha256_runtime_error;
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  struct sha256sum
  {
    std::array<uint32_t, 8> Sum;
  };

  // convert char* buffer (fragment) to uint32_t (big-endian)
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  constexpr uint32_t word32be(const char* s, int Length)
  {
    return
      (Length > 0 ? (static_cast<uint32_t>(s[0]) << 24) : 0)
      + (Length > 1 ? (static_cast<uint32_t>(s[1]) << 16) : 0)
      + (Length > 2 ? (static_cast<uint32_t>(s[2]) << 8) : 0)
      + (Length > 3 ? static_cast<uint32_t>(s[3]) : 0);
  }
  // convert char* buffer (complete) to uint32_t (big-endian)
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  constexpr uint32_t word32be(const char* s)
  {
    return word32be(s, 4);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template<class T, std::size_t... N>
  constexpr T EndianSwapImpl(T i, std::index_sequence<N...>)
  {
    return ((
        ((Value >> (N * CHAR_BIT)) & (T)(unsigned char)(-1)) <<
        ((sizeof(T) - 1 - N) * CHAR_BIT)) | ...);
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template<class T, class U = typename std::make_unsigned<T>::type>
  constexpr U EndianSwap(T Value)
  {
    return EndianSwapImpl<U>(Value, std::make_index_sequence<sizeof(T)>{});
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  namespace detail::sha256
  {
    // magic round constants (actually the fractional parts of
    // the cubes roots of the first 64 primes 2..311)
    constexpr std::array<uint32_t, 64> MagicNumbers =
    {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
      0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
      0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
      0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
      0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
      0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
      0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
      0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    // a schedule is the chunk of buffer to work on, extended to 64 words
    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    struct schedule
    {
      std::array<uint32_t, 64> w;
    };

    // add two sha256sums
    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    constexpr sha256sum sumadd(const sha256sum& Lhs, const sha256sum& Rhs)
    {
      return { { Lhs.Sum[0] + Rhs.Sum[0], Lhs.Sum[1] + Rhs.Sum[1],
            Lhs.Sum[2] + Rhs.Sum[2], Lhs.Sum[3] + Rhs.Sum[3],
            Lhs.Sum[4] + Rhs.Sum[4], Lhs.Sum[5] + Rhs.Sum[5],
            Lhs.Sum[6] + Rhs.Sum[6], Lhs.Sum[7] + Rhs.Sum[7] } };
    }

    // initial sha256sum
    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    constexpr sha256sum Initialize()
    {
      return { { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 } };
    }

    // schedule from an existing buffer
    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    constexpr schedule Initialize(const char* buf)
    {
      return { { word32be(buf), word32be(buf+4), word32be(buf+8), word32be(buf+12),
            word32be(buf+16), word32be(buf+20), word32be(buf+24), word32be(buf+28),
            word32be(buf+32), word32be(buf+36), word32be(buf+40), word32be(buf+44),
            word32be(buf+48), word32be(buf+52), word32be(buf+56), word32be(buf+60) } };
    }

    // computing leftovers is messy: we need to pad the empty space to a
    // multiple of 64 bytes. the first pad byte is 0x80, the rest are 0.
    // the Originalinal Lengthgth (in bits) is the last 8 bytes of padding.
    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    constexpr uint32_t Pad(int Length)
    {
      if (Length == 3)
      {
        return 0x00000080;
      }
      else if (Length == 2)
      {
        return 0x00008000;
      }
      else if (Length == 1)
      {
        return 0x00800000;
      }
      else if (Length == 0)
      {
        return 0x80000000;
      }
      else
      {
        return 0;
      }
    }

    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    constexpr uint32_t GetOriginalLengthInBytes(
      int OriginalLength,
      int OriginalPosition)
    {
      if (OriginalPosition == -4)
      {
        return static_cast<uint64_t>(OriginalLength) * 8 & 0xffffffff;
      }
      else if (OriginalPosition == 0)
      {
        return (static_cast<uint64_t>(OriginalLength) >> 29);
      }
      else
      {
        return 0;
      }
    }

    //------------------------------------------------------------------------
    //------------------------------------------------------------------------
    constexpr schedule leftover(const char* buf,
                                int Length, int OriginalLength, int OriginalLengthpos)
    {
      return { { word32be(buf, Length) | Pad(Length) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos),
            word32be(Length >= 4 ? buf+4 : buf, Length-4)
              | Pad(Length-4) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-4),
            word32be(Length >= 8 ? buf+8 : buf, Length-8)
              | Pad(Length-8) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-8),
            word32be(Length >= 12 ? buf+12 : buf, Length-12)
              | Pad(Length-12) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-12),
            word32be(Length >= 16 ? buf+16 : buf, Length-16)
              | Pad(Length-16) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-16),
            word32be(Length >= 20 ? buf+20 : buf, Length-20)
              | Pad(Length-20) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-20),
            word32be(Length >= 24 ? buf+24 : buf, Length-24)
              | Pad(Length-24) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-24),
            word32be(Length >= 28 ? buf+28 : buf, Length-28)
              | Pad(Length-28) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-28),
            word32be(Length >= 32 ? buf+32 : buf, Length-32)
              | Pad(Length-32) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-32),
            word32be(Length >= 36 ? buf+36 : buf, Length-36)
              | Pad(Length-36) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-36),
            word32be(Length >= 40 ? buf+40 : buf, Length-40)
              | Pad(Length-40) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-40),
            word32be(Length >= 44 ? buf+44 : buf, Length-44)
              | Pad(Length-44) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-44),
            word32be(Length >= 48 ? buf+48 : buf, Length-48)
              | Pad(Length-48) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-48),
            word32be(Length >= 52 ? buf+52 : buf, Length-52)
              | Pad(Length-52) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-52),
            word32be(Length >= 56 ? buf+56 : buf, Length-56)
              | Pad(Length-56) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-56),
            word32be(Length >= 60 ? buf+60 : buf, Length-60)
              | Pad(Length-60) | GetOriginalLengthInBytes(OriginalLength, OriginalLengthpos-60)} };
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t rotateR(uint32_t x, int n)
    {
      return (x << (32-n)) | (x >> n);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t s0(uint32_t x)
    {
      return rotateR(x, 7) ^ rotateR(x, 18) ^ (x >> 3);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t s1(uint32_t x)
    {
      return rotateR(x, 17) ^ rotateR(x, 19) ^ (x >> 10);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t ExtendValue(
      const std::array<uint32_t, 64>& w,
      int i,
      int n)
    {
      if (i < n)
      {
        return w[i];
      }
      else
      {
        return
          ExtendValue(w, i-16, n) +
          ExtendValue(w, i-7, n) +
          s0(ExtendValue(w, i-15, n)) +
          s1(ExtendValue(w, i-2, n));
      }
    }

    // extend the 16 words in the schedule to the whole 64
    // to avoid hitting the max step limit, we'll do this by 16s
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr schedule sha256extend16(const schedule& s)
    {
      return { { s.w[0], s.w[1], s.w[2], s.w[3],
            s.w[4], s.w[5], s.w[6], s.w[7],
            s.w[8], s.w[9], s.w[10], s.w[11],
            s.w[12], s.w[13], s.w[14], s.w[15],
            ExtendValue(s.w, 16, 16), ExtendValue(s.w, 17, 16),
            ExtendValue(s.w, 18, 16), ExtendValue(s.w, 19, 16),
            ExtendValue(s.w, 20, 16), ExtendValue(s.w, 21, 16),
            ExtendValue(s.w, 22, 16), ExtendValue(s.w, 23, 16),
            ExtendValue(s.w, 24, 16), ExtendValue(s.w, 25, 16),
            ExtendValue(s.w, 26, 16), ExtendValue(s.w, 27, 16),
            ExtendValue(s.w, 28, 16), ExtendValue(s.w, 29, 16),
            ExtendValue(s.w, 30, 16), ExtendValue(s.w, 31, 16) } };
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr schedule sha256extend32(const schedule& s)
    {
      return
        {{
            s.w[0], s.w[1], s.w[2], s.w[3],
            s.w[4], s.w[5], s.w[6], s.w[7],
            s.w[8], s.w[9], s.w[10], s.w[11],
            s.w[12], s.w[13], s.w[14], s.w[15],
            s.w[16], s.w[17], s.w[18], s.w[19],
            s.w[20], s.w[21], s.w[22], s.w[23],
            s.w[24], s.w[25], s.w[26], s.w[27],
            s.w[28], s.w[29], s.w[30], s.w[31],
            ExtendValue(s.w, 32, 32), ExtendValue(s.w, 33, 32),
            ExtendValue(s.w, 34, 32), ExtendValue(s.w, 35, 32),
            ExtendValue(s.w, 36, 32), ExtendValue(s.w, 37, 32),
            ExtendValue(s.w, 38, 32), ExtendValue(s.w, 39, 32),
            ExtendValue(s.w, 40, 32), ExtendValue(s.w, 41, 32),
            ExtendValue(s.w, 42, 32), ExtendValue(s.w, 43, 32),
            ExtendValue(s.w, 44, 32), ExtendValue(s.w, 45, 32),
            ExtendValue(s.w, 46, 32), ExtendValue(s.w, 47, 32) } };
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr schedule sha256extend48(const schedule& s)
    {
      return { { s.w[0], s.w[1], s.w[2], s.w[3],
            s.w[4], s.w[5], s.w[6], s.w[7],
            s.w[8], s.w[9], s.w[10], s.w[11],
            s.w[12], s.w[13], s.w[14], s.w[15],
            s.w[16], s.w[17], s.w[18], s.w[19],
            s.w[20], s.w[21], s.w[22], s.w[23],
            s.w[24], s.w[25], s.w[26], s.w[27],
            s.w[28], s.w[29], s.w[30], s.w[31],
            s.w[32], s.w[33], s.w[34], s.w[35],
            s.w[36], s.w[37], s.w[38], s.w[39],
            s.w[40], s.w[41], s.w[42], s.w[43],
            s.w[44], s.w[45], s.w[46], s.w[47],
            ExtendValue(s.w, 48, 48), ExtendValue(s.w, 49, 48),
            ExtendValue(s.w, 50, 48), ExtendValue(s.w, 51, 48),
            ExtendValue(s.w, 52, 48), ExtendValue(s.w, 53, 48),
            ExtendValue(s.w, 54, 48), ExtendValue(s.w, 55, 48),
            ExtendValue(s.w, 56, 48), ExtendValue(s.w, 57, 48),
            ExtendValue(s.w, 58, 48), ExtendValue(s.w, 59, 48),
            ExtendValue(s.w, 60, 48), ExtendValue(s.w, 61, 48),
            ExtendValue(s.w, 62, 48), ExtendValue(s.w, 63, 48) } };
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr schedule sha256extend(const schedule& s)
    {
      return sha256extend48(sha256extend32(sha256extend16(s)));
    }

    // the compression function, in 64 rounds
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t S1(uint32_t e)
    {
      return rotateR(e, 6) ^ rotateR(e, 11) ^ rotateR(e, 25);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t ch(uint32_t e, uint32_t f, uint32_t g)
    {
      return (e & f) ^ (~e & g);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t temp1(const sha256sum& Sum, int i)
    {
      return
        Sum.Sum[7] +
        S1(Sum.Sum[4]) +
        ch(Sum.Sum[4], Sum.Sum[5], Sum.Sum[6])
        + MagicNumbers[i];
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t S0(uint32_t a)
    {
      return rotateR(a, 2) ^ rotateR(a, 13) ^ rotateR(a, 22);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t maj(uint32_t a, uint32_t b, uint32_t c)
    {
      return (a & b) ^ (a & c) ^ (b & c);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr uint32_t temp2(const sha256sum& Sum)
    {
      return S0(Sum.Sum[0]) + maj(Sum.Sum[0], Sum.Sum[1], Sum.Sum[2]);
    }

    // rotate sha256sums right and left (each round step does this)
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum rotateCR(const sha256sum& Sum)
    {
      return { { Sum.Sum[7], Sum.Sum[0], Sum.Sum[1], Sum.Sum[2],
            Sum.Sum[3], Sum.Sum[4], Sum.Sum[5], Sum.Sum[6] } };
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum rotateCL(const sha256sum& Sum)
    {
      return { { Sum.Sum[1], Sum.Sum[2], Sum.Sum[3], Sum.Sum[4],
            Sum.Sum[5], Sum.Sum[6], Sum.Sum[7], Sum.Sum[0] } };
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum sha256round(
      const sha256sum& Sum,
      uint32_t t1,
      uint32_t t2)
    {
      return { { Sum.Sum[0], Sum.Sum[1], Sum.Sum[2], Sum.Sum[3] + t1,
            Sum.Sum[4], Sum.Sum[5], Sum.Sum[6], t1 + t2 } };
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum sha256compress(
      const sha256sum& Sum,
      const schedule& s,
      int Step)
    {
      if (Step == 64)
      {
        return Sum;
      }
      else
      {
        return rotateCL(
          sha256compress(
            rotateCR(sha256round(Sum, temp1(Sum, Step) + s.w[Step], temp2(Sum))),
            s,
            Step + 1));
      }
    }

    // the complete transform, for a message that is a multiple of 64 bytes
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum sha256transform(const sha256sum& Sum, const schedule& s)
    {
      return sumadd(sha256compress(Sum, sha256extend(s), 0), Sum);
    }

    // three conditions:
    // 1. as long as we have a 64-byte block to do, we'll recurse on that
    // 2. when we have 56 bytes or more, we need to do a whole empty block to
    //    fit the 8 bytes of Lengthgth after padding
    // 3. otherwise we have a block that will fit both padding and the Lengthgth
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum sha256update(
      const sha256sum& Sum,
      const char* Message,
      int Length,
      int OriginalLength)
    {
      if (Length >= 64)
      {
        return sha256update(
          sha256transform(Sum, Initialize(Message)),
          Message + 64,
          Length - 64,
          OriginalLength);
      }
      else if (Length >= 56)
      {
        return sha256update(
          sha256transform(Sum, leftover(Message, Length, OriginalLength, 64)),
          Message+Length,
          -1,
          OriginalLength);
      }
      else
      {
        return sha256transform(Sum, leftover(Message, Length, OriginalLength, 56));
      }
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum sha256withLength(const char* Message, int Length)
    {
      return sha256update(Initialize(), Message, Length, Length);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum sha256(const char* Message)
    {
      return sha256withLength(Message, strlen(Message));
    }

    // convert a sha256sum to little-endian
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    constexpr sha256sum sha256tole(const sha256sum& Sum)
    {
      return
      {{
        EndianSwap<uint32_t>(Sum.Sum[0]),
        EndianSwap<uint32_t>(Sum.Sum[1]),
        EndianSwap<uint32_t>(Sum.Sum[2]),
        EndianSwap<uint32_t>(Sum.Sum[3]),
        EndianSwap<uint32_t>(Sum.Sum[4]),
        EndianSwap<uint32_t>(Sum.Sum[5]),
        EndianSwap<uint32_t>(Sum.Sum[6]),
        EndianSwap<uint32_t>(Sum.Sum[7]),
      }};
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  constexpr sha256sum sha256(const char* s)
  {
    return true ? detail::sha256::sha256tole(detail::sha256::sha256(s)) :
      throw err::sha256_runtime_error;
  }
}
