#ifndef GAME_ENGINE_FILE_H
#define GAME_ENGINE_FILE_H

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace game_engine {

struct FileType {
  enum Enum {
    Binary = 0,
    Text,
  };
};

struct ReadWriteType {
  enum Enum {
    Read = 0,  // READ ONLY, MUST EXIST FILE
    Write,   // WRITE ONLY, IF THE FILE EXIST, DELETE ALL CONTENTS, IF THE FILE
             // NOT EXIST CREATE FILE.
    Append,  // WRITE ONLY END OF THE FILE, IF THE FILE NOT EXIST CREATE FILE.
    ReadUpdate,    // READ / WRITE, IF THE FILE NOT EXIST, NULL WILL RETURN
    WriteUpdate,   // READ / WRITE, IF THE FILE EXIST, DELETE ALL CONTENTS, IF
                    // THE FILE NOT EXIST, FILE WILL CREATE.
    AppendUpdate,  // READ / WRITE, WRITE END OF THE FILE, READ CAN ANY WHERE.
                    // IF THE FILE NOT EXIST, FILE WILL CREATE
  };
};

class File {
  public:
  typedef char                     ElementType;
  typedef std::vector<ElementType> FileBuffer;

  static uint64_t s_getFileTimeStamp(const std::string& filename);

  File()
      : m_fp_(nullptr) {}

  ~File();

  bool openFile(const std::string&  szFileName,
                FileType::Enum      fileType      = FileType::Binary,
                ReadWriteType::Enum readWriteType = ReadWriteType::Read);

  size_t readFileToBuffer(bool   appendToEndofBuffer = true,
                          size_t index               = 0,
                          size_t count               = 0);

  void closeFile();

  const char* getBuffer(size_t index = 0, size_t count = 0) const;

  bool getBuffer(FileBuffer&        buffer,
                 const std::string& startToken,
                 const std::string& endToken);

  // TODO: seems not used
  bool isBufferEmpty() const { return m_buffer_.empty(); }

  private:
  std::FILE*               m_fp_;
  std::vector<ElementType> m_buffer_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FILE_H