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
  // ======= BEGIN: public static methods =====================================

  template <typename T2>
  static void s_getHash(size_t& hash, int32_t index, const T2& data) {
    hash ^= (data->getHash() << index);
  }

  size_t getHash() const {
    size_t hash = 0;
    for (int32_t i = 0; i < m_numOfData_; ++i) {
      // hash ^= (Data[i]->s_getHash() << i);
      s_getHash(hash, i, m_data_[i]);
    }
    return hash;
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public constructors =======================================

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

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc methods =======================================

  void add(T element) {
    assert(NumOfInlineData > m_numOfData_);

    m_data_[m_numOfData_] = element;
    ++m_numOfData_;
  }

  void add(void* elementAddress, int32_t count) {
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

  void popBack() {
    if (m_numOfData_ > 0) {
      --m_numOfData_;
    }
  }

  const T& back() {
    if (m_numOfData_ > 0) {
      return m_data_[m_numOfData_ - 1];
    }
    static auto EmptyValue = T();
    return EmptyValue;
  }

  void reset() { m_numOfData_ = 0; }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public overloaded operators ===============================

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

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public misc fields ========================================

  T       m_data_[NumOfInlineData];
  int32_t m_numOfData_ = 0;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RESOURCE_CONTAINER_H
