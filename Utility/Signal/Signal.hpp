#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <vector>

namespace dl
{
  template <typename ... T>
  class Signal
  {
    public:

      template <typename SlotType>
      void Connect(SlotType&& Slot) const;

      template <class ... ArgsType>
      void operator()(ArgsType&& ... Args);

      size_t GetConnectedSlotCount() const;

    private:

      mutable std::mutex mMutex;

      mutable std::vector<std::function<void(T...)>> mSlots;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename ... T>
  template <typename SlotType>
  void Signal<T...>::Connect(SlotType&& Slot) const
  {
    std::lock_guard<std::mutex> LockGuard(mMutex);

    mSlots.emplace_back(std::forward<SlotType>(Slot));
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename ... T>
  template<typename ... ArgsType>
  void Signal<T...>::operator()(ArgsType&& ... Args)
  {
    std::lock_guard<std::mutex> LockGuard(mMutex);

    for(auto& Slot : mSlots)
    {
      Slot(Args...);
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename ... T>
  size_t Signal<T...>::GetConnectedSlotCount() const
  {
    std::lock_guard<std::mutex> LockGuard(mMutex);

    return mSlots.size();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <>
  class Signal<void>
  {
    public:
      template <typename SlotType>
      void Connect(SlotType&& Slot) const;

      template <class ... ArgsType>
      void operator()(ArgsType&& ... Args);

      size_t GetConnectedSlotCount();

    private:

      mutable std::mutex mMutex;

      mutable std::vector<std::function<void(void)>> mSlots;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template<typename SlotType>
  void Signal<void>::Connect(SlotType&& Slot) const
  {
    std::lock_guard<std::mutex> LockGuard(mMutex);

    mSlots.emplace_back(std::forward<SlotType>(Slot));
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template<typename ... ArgsType>
  void Signal<void>::operator()(ArgsType&& ... Args)
  {
    std::lock_guard<std::mutex> LockGuard(mMutex);

    for(auto& Slot : mSlots)
    {
      Slot();
    }
  }
}
