#include "file_loader/file.h"

#include <cassert>

namespace game_engine {

uint64_t jFile::GetFileTimeStamp(const std::string& filename) {
  assert(!filename.empty());
  auto lastWriteTime = std::filesystem::last_write_time(filename);
  return static_cast<uint64_t>(lastWriteTime.time_since_epoch().count());
}

jFile::~jFile() {
  CloseFile();
}

bool jFile::OpenFile(const std::string&  szFileName,
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
  m_fp = std::fopen(szFileName.c_str(), option.c_str());
  return m_fp != nullptr;
}

size_t jFile::ReadFileToBuffer(bool   appendToEndofBuffer,
                               size_t index,
                               size_t count) {
  size_t readSize = 0;

  if (m_fp) {
    size_t writeStartIndex = 0;

    if (count > 0) {
      if (appendToEndofBuffer) {
        writeStartIndex = m_buffer.size();
        m_buffer.resize(m_buffer.size() + count);
      } else {
        m_buffer.resize(count);
      }
    } else {
      std::fseek(m_fp, 0, SEEK_END);
      count = std::ftell(m_fp);
      m_buffer.clear();
      m_buffer.resize(count);
    }

    std::fseek(m_fp, static_cast<long>(index), SEEK_SET);
    readSize = std::fread(
        &m_buffer[writeStartIndex], sizeof(ELEMENT_TYPE), count, m_fp);
    m_buffer.resize(writeStartIndex + readSize);
  }

  return readSize;
}

void jFile::CloseFile() {
  if (m_fp) {
    std::fclose(m_fp);
    m_fp = nullptr;
  }
}

const char* jFile::GetBuffer(size_t index, size_t count) const {
  if (m_buffer.empty()) {
    return nullptr;
  }
  return &m_buffer[0];
}

bool jFile::GetBuffer(FILE_BUFFER&       outBuffer,
                      const std::string& startToken,
                      const std::string& endToken) {
  if (m_buffer.empty()) {
    return false;
  }

  const char* startPos = std::strstr(&m_buffer[0], startToken.c_str());
  if (startPos) {
    startPos           += startToken.length();
    const char* endPos  = std::strstr(startPos, endToken.c_str());
    size_t      count   = 0;

    if (endPos) {
      count = endPos - startPos;
    } else {
      count = &m_buffer[m_buffer.size()] - startPos;
    }

    if (count > 0) {
      outBuffer.resize(count);
      std::memcpy(&outBuffer[0], startPos, count * sizeof(ELEMENT_TYPE));
      return true;
    }
  }

  return false;
}

}  // namespace game_engine