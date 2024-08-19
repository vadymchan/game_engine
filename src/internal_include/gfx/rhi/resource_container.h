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

  ResourceContainer(const ResourceContainer& data) {
    // TODO: remove (replaced memcpy to std::copy_n
    // if constexpr (std::is_trivially_copyable<T>::value) {
    //   memcpy(&Data[0], &data.Data[0], sizeof(T) * data.NumOfData);
    // } else {
    //   for (int32_t i = 0; i < data.NumOfData; ++i) {
    //     Data[i] = data[i];
    //   }
    // }
    std::copy_n(data.m_data_, data.m_numOfData_, m_data_);
    m_numOfData_ = data.m_numOfData_;
  }

  void Add(T element) {
    assert(NumOfInlineData > m_numOfData_);

    m_data_[m_numOfData_] = element;
    ++m_numOfData_;
  }

  void Add(void* elementAddress, int32_t count) {
    assert(NumOfInlineData > (m_numOfData_ + count));
    assert(elementAddress);
    // TODO: remove (replaced memcpy to std::copy_n
    /*if constexpr (std::is_trivially_copyable<T>::value) {
      memcpy(&Data[NumOfData], elementAddress, count * sizeof(T));
    } else {
      for (int32_t i = 0; i < count; ++i) {
        Data[NumOfData + i] = *((T*)elementAddress[i]);
      }
    }*/
    std::copy_n(
        static_cast<const T*>(elementAddress), count, m_data_ + m_numOfData_);
    m_numOfData_ += count;
  }

  void PopBack() {
    if (m_numOfData_ > 0) {
      --m_numOfData_;
    }
  }

  const T& Back() {
    if (m_numOfData_ > 0) {
      return m_data_[m_numOfData_ - 1];
    }
    static auto EmptyValue = T();
    return EmptyValue;
  }

  template <typename T2>
  static void GetHash(size_t& hash, int32_t index, const T2& data) {
    hash ^= (data->GetHash() << index);
  }

  size_t GetHash() const {
    size_t Hash = 0;
    for (int32_t i = 0; i < m_numOfData_; ++i) {
      // Hash ^= (Data[i]->GetHash() << i);
      GetHash(Hash, i, m_data_[i]);
    }
    return Hash;
  }

  void Reset() { m_numOfData_ = 0; }

  ResourceContainer<T, NumOfInlineData>& operator=(
      const ResourceContainer<T, NumOfInlineData>& data) {
    // TODO: remove (replaced memcpy to std::copy_n
    // if constexpr (std::is_trivially_copyable<T>::value) {
    //  memcpy(&Data[0], &data.Data[0], sizeof(T) * data.NumOfData);
    //} else {
    //  for (int32_t i = 0; i < data.NumOfData; ++i) {
    //    Data[i] = data[i];
    //  }
    //}
    // NumOfData = data.NumOfData;
    if (this != &data) {
      std::copy_n(data.m_data_, data.m_numOfData_, m_data_);
      m_numOfData_ = data.m_numOfData_;
    }
    return *this;
  }

  T& operator[](int32_t index) {
    assert(index < m_numOfData_);
    return m_data_[index];
  }

  const T& operator[](int32_t index) const {
    assert(index < m_numOfData_);
    return m_data_[index];
  }

  T       m_data_[NumOfInlineData];
  int32_t m_numOfData_ = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RESOURCE_CONTAINER_H
