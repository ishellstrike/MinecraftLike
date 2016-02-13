// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef CoordsConvert_h__
#define CoordsConvert_h__

#include <glm/glm.hpp>
#include "../Core/Config.h"

typedef glm::vec3   WPos;     // ������� ����������.
typedef glm::ivec3  SPos;     // ���������� �������.
typedef glm::ivec3  WBPos;    // ���������� ����� � ����.
typedef glm::ivec3  SBPos;    // ���������� ����� � �������.


/// ������� ���������.
namespace cs
{
  namespace detail
  {
    template<class T>
    inline float move(float val)
    {
      return val + static_cast<T> (1.0f - (static_cast<T> (val) - val));
    }
  }

  /// ������� ���������� � ���������� ����� � ����.
  inline WBPos WtoWB(const WPos &pos)
  {
    WBPos wbpos;
    typedef decltype(wbpos)::value_type vtype;

    wbpos.x = (pos.x >= 0) ? static_cast<vtype>(pos.x) : static_cast<vtype>(detail::move<vtype>(pos.x)) - vtype(1);
    wbpos.y = (pos.y >= 0) ? static_cast<vtype>(pos.y) : static_cast<vtype>(detail::move<vtype>(pos.y)) - vtype(1);
    wbpos.z = (pos.z >= 0) ? static_cast<vtype>(pos.z) : static_cast<vtype>(detail::move<vtype>(pos.z)) - vtype(1);

    return wbpos;
  }

  /// ���������� ����� � ���� � ���������� �������.
  inline SPos WBtoS(const WBPos &pos)
  {
    SPos spos;
    typedef decltype(spos)::value_type vtype;
    const vtype size = static_cast<vtype>(SECTOR_SIZE);

    spos.x = (pos.x >= 0) ? static_cast<vtype>(pos.x) / size : (static_cast<vtype>(pos.x) - size + vtype(1)) / size;
    spos.y = (pos.y >= 0) ? static_cast<vtype>(pos.y) / size : (static_cast<vtype>(pos.y) - size + vtype(1)) / size;
    spos.z = (pos.z >= 0) ? static_cast<vtype>(pos.z) / size : (static_cast<vtype>(pos.z) - size + vtype(1)) / size;

    return spos;
  }

  /// ������� ���������� � ���������� �������.
  inline SPos WtoS(const WPos &pos)
  {
    return WBtoS(WtoWB(pos));
  }

  /// ������� ���������� � ���������� ����� � �������.
  inline SBPos WtoSB(const WPos &pos)
  {
    auto wbpos = WtoWB(pos);
    auto spos = WBtoS(wbpos);
    typedef decltype(spos)::value_type vtype;
    const vtype size = static_cast<vtype>(SECTOR_SIZE);

    return wbpos - spos * size;
  }

  /// ������� ���������� � ���������� ����� � �������.
  inline SBPos WtoSB(const WPos &pos, const SPos &spos)
  {
    typedef SBPos::value_type vtype;
    const vtype size = static_cast<vtype>(SECTOR_SIZE);

    return WtoWB(pos) - spos * size;
  }

  /// ���������� ����� � ���� � ���������� ����� � �������.
  inline SBPos WBtoSB(const WBPos &pos)
  {
    auto spos = WBtoS(pos);
    typedef decltype(spos)::value_type vtype;
    const vtype size = static_cast<vtype>(SECTOR_SIZE);

    return pos - spos * size;
  }

  /// ���������� ����� � ���� � ���������� ����� � �������.
  inline SBPos WBtoSB(const WBPos &pos, const SPos &spos)
  {
    typedef SBPos::value_type vtype;
    const vtype size = static_cast<vtype>(SECTOR_SIZE);

    return pos - spos * size;
  }

  /// ���������� ����� � ������� � ���������� ����� � ����.
  inline WBPos SBtoWB(const SBPos &pos, const SPos &spos)
  {
    typedef WBPos::value_type vtype;
    const vtype size = static_cast<vtype>(SECTOR_SIZE);

    return spos * size + pos;
  }

  /// ���������� ������� � ������� ����������.
  /// ������������ ���������� �������� ����� � �������.
  inline WPos StoW(const SPos &pos)
  {
    WPos wpos;
    typedef SPos::value_type vtype;
    const vtype size = static_cast<vtype>(SECTOR_SIZE);

    return pos * size;
  }
}

namespace std
{
  template <>
  struct hash<SPos>
  {
    std::size_t operator()(SPos const& v) const
    {
      std::size_t h1 = std::hash<int32_t>()(v.x);
      std::size_t h2 = std::hash<int32_t>()(v.y);
      std::size_t h3 = std::hash<int32_t>()(v.z);
      return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
    }
  };
}

#endif // CoordsConvert_h__