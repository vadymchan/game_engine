#ifndef GAME_ENGINE_RESOURCE_CONTAINER_H
#define GAME_ENGINE_RESOURCE_CONTAINER_H

#include <algorithm>
#include <cassert>
#include <type_traits>

namespace game_engine {

// TODO: move file to other folder (e.g. utils)

// Temporary array for one frame data
template <typename T, int32_t NumOfInlineData = 10>
struct ResourceContainer {
  ResourceContainer() = default;

  ResourceContainer(const ResourceContainer& InData) {
    // TODO: remove (replaced memcpy to std::copy_n
    // if constexpr (std::is_trivially_copyable<T>::value) {
    //   memcpy(&Data[0], &InData.Data[0], sizeof(T) * InData.NumOfData);
    // } else {
    //   for (int32_t i = 0; i < InData.NumOfData; ++i) {
    //     Data[i] = InData[i];
    //   }
    // }
    std::copy_n(InData.Data, InData.NumOfData, Data);
    NumOfData = InData.NumOfData;
  }

  void Add(T InElement) {
    assert(NumOfInlineData > NumOfData);

    Data[NumOfData] = InElement;
    ++NumOfData;
  }

  void Add(void* InElementAddress, int32_t InCount) {
    assert(NumOfInlineData > (NumOfData + InCount));
    assert(InElementAddress);
    // TODO: remove (replaced memcpy to std::copy_n
    /*if constexpr (std::is_trivially_copyable<T>::value) {
      memcpy(&Data[NumOfData], InElementAddress, InCount * sizeof(T));
    } else {
      for (int32_t i = 0; i < InCount; ++i) {
        Data[NumOfData + i] = *((T*)InElementAddress[i]);
      }
    }*/
    std::copy_n(
        static_cast<const T*>(InElementAddress), InCount, Data + NumOfData);
    NumOfData += InCount;
  }

  void PopBack() {
    if (NumOfData > 0) {
      --NumOfData;
    }
  }

  const T& Back() {
    if (NumOfData > 0) {
      return Data[NumOfData - 1];
    }
    static auto EmptyValue = T();
    return EmptyValue;
  }

  template <typename T2>
  static void GetHash(size_t& InOutHash, int32_t index, const T2& InData) {
    InOutHash ^= (InData->GetHash() << index);
  }

  size_t GetHash() const {
    size_t Hash = 0;
    for (int32_t i = 0; i < NumOfData; ++i) {
      // Hash ^= (Data[i]->GetHash() << i);
      GetHash(Hash, i, Data[i]);
    }
    return Hash;
  }

  void Reset() { NumOfData = 0; }

  ResourceContainer<T, NumOfInlineData>& operator=(
      const ResourceContainer<T, NumOfInlineData>& InData) {
    // TODO: remove (replaced memcpy to std::copy_n
    // if constexpr (std::is_trivially_copyable<T>::value) {
    //  memcpy(&Data[0], &InData.Data[0], sizeof(T) * InData.NumOfData);
    //} else {
    //  for (int32_t i = 0; i < InData.NumOfData; ++i) {
    //    Data[i] = InData[i];
    //  }
    //}
    // NumOfData = InData.NumOfData;
    if (this != &InData) {
      std::copy_n(InData.Data, InData.NumOfData, Data);
      NumOfData = InData.NumOfData;
    }
    return *this;
  }

  T& operator[](int32_t InIndex) {
    assert(InIndex < NumOfData);
    return Data[InIndex];
  }

  const T& operator[](int32_t InIndex) const {
    assert(InIndex < NumOfData);
    return Data[InIndex];
  }

  T       Data[NumOfInlineData];
  int32_t NumOfData = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RESOURCE_CONTAINER_H
