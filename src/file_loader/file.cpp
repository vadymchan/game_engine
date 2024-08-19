#include "file_loader/file.h"

#include <cassert>

namespace game_engine {

uint64_t File::GetFileTimeStamp(const std::string& filename) {
  assert(!filename.empty());
  auto lastWriteTime = std::filesystem::last_write_time(filename);
  return static_cast<uint64_t>(lastWriteTime.time_since_epoch().count());
}

File::~File() {
  CloseFile();
}

bool File::OpenFile(const std::string&  szFileName,
                     FileType::Enum      fileType,
                     ReadWriteType::Enum readWriteType) {
  std::string option;

  switch (readWriteType) {
    case ReadWriteType::READ:
      option += "r";
      break;
    case ReadWriteType::WRITE:
      option += "w";
      break;
    case ReadWriteType::APPEND:
      option += "a";
      break;
    case ReadWriteType::READ_UPDATE:
      option += "r+";
      break;
    case ReadWriteType::WRITE_UPDATE:
      option += "w+";
      break;
    case ReadWriteType::APPEND_UPDATE:
      option += "a+";
      break;
  }

  switch (fileType) {
    case FileType::BINARY:
      option += "b";
      break;
    case FileType::TEXT:
      option += "t";
      break;
  }

  CloseFile();
  m_fp_ = std::fopen(szFileName.c_str(), option.c_str());
  return m_fp_ != nullptr;
}

size_t File::ReadFileToBuffer(bool   appendToEndofBuffer,
                               size_t index,
                               size_t count) {
  size_t readSize = 0;

  if (m_fp_) {
    size_t writeStartIndex = 0;

    if (count > 0) {
      if (appendToEndofBuffer) {
        writeStartIndex = m_buffer_.size();
        m_buffer_.resize(m_buffer_.size() + count);
      } else {
        m_buffer_.resize(count);
      }
    } else {
      std::fseek(m_fp_, 0, SEEK_END);
      count = std::ftell(m_fp_);
      m_buffer_.clear();
      m_buffer_.resize(count);
    }

    std::fseek(m_fp_, static_cast<long>(index), SEEK_SET);
    readSize = std::fread(
        &m_buffer_[writeStartIndex], sizeof(ELEMENT_TYPE), count, m_fp_);
    m_buffer_.resize(writeStartIndex + readSize);
  }

  return readSize;
}

void File::CloseFile() {
  if (m_fp_) {
    std::fclose(m_fp_);
    m_fp_ = nullptr;
  }
}

const char* File::GetBuffer(size_t index, size_t count) const {
  if (m_buffer_.empty()) {
    return nullptr;
  }
  return &m_buffer_[0];
}

bool File::GetBuffer(FILE_BUFFER&       buffer,
                      const std::string& startToken,
                      const std::string& endToken) {
  if (m_buffer_.empty()) {
    return false;
  }

  const char* startPos = std::strstr(&m_buffer_[0], startToken.c_str());
  if (startPos) {
    startPos           += startToken.length();
    const char* endPos  = std::strstr(startPos, endToken.c_str());
    size_t      count   = 0;

    if (endPos) {
      count = endPos - startPos;
    } else {
      count = &m_buffer_[m_buffer_.size()] - startPos;
    }

    if (count > 0) {
      buffer.resize(count);
      std::memcpy(&buffer[0], startPos, count * sizeof(ELEMENT_TYPE));
      return true;
    }
  }

  return false;
}

}  // namespace game_engine