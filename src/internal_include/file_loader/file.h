#ifndef GAME_ENGINE_FILE_H
#define GAME_ENGINE_FILE_H

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace game_engine {

struct FileType {
  enum Enum {
    BINARY = 0,
    TEXT,
  };
};

struct ReadWriteType {
  enum Enum {
    READ = 0,  // READ ONLY, MUST EXIST FILE
    WRITE,   // WRITE ONLY, IF THE FILE EXIST, DELETE ALL CONTENTS, IF THE FILE
             // NOT EXIST CREATE FILE.
    APPEND,  // WRITE ONLY END OF THE FILE, IF THE FILE NOT EXIST CREATE FILE.
    READ_UPDATE,    // READ / WRITE, IF THE FILE NOT EXIST, NULL WILL RETURN
    WRITE_UPDATE,   // READ / WRITE, IF THE FILE EXIST, DELETE ALL CONTENTS, IF
                    // THE FILE NOT EXIST, FILE WILL CREATE.
    APPEND_UPDATE,  // READ / WRITE, WRITE END OF THE FILE, READ CAN ANY WHERE.
                    // IF THE FILE NOT EXIST, FILE WILL CREATE
  };
};

class File {
  public:
  typedef char                      ELEMENT_TYPE;
  typedef std::vector<ELEMENT_TYPE> FILE_BUFFER;

static uint64_t GetFileTimeStamp(const std::string& filename);

  File()
      : m_fp(nullptr) {}

  ~File();

  bool OpenFile(const std::string&  szFileName,
                FileType::Enum      fileType      = FileType::BINARY,
                ReadWriteType::Enum readWriteType = ReadWriteType::READ);

  size_t ReadFileToBuffer(bool   appendToEndofBuffer = true,
                          size_t index               = 0,
                          size_t count               = 0);

  void CloseFile();

  const char* GetBuffer(size_t index = 0, size_t count = 0) const;

  bool GetBuffer(FILE_BUFFER&       outBuffer,
                 const std::string& startToken,
                 const std::string& endToken);

  bool IsBufferEmpty() const { return m_buffer.empty(); }

  private:
  std::FILE*                m_fp;
  std::vector<ELEMENT_TYPE> m_buffer;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FILE_H