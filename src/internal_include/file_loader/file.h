#ifndef GAME_ENGINE_FILE_H
#define GAME_ENGINE_FILE_H

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace game_engine {

struct FileType {
  // ======= BEGIN: public nested types =======================================

  enum Enum {
    Binary = 0,
    Text,
  };

  // ======= END: public nested types   =======================================
};

struct ReadWriteType {
  // ======= BEGIN: public nested types =======================================

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

  // ======= END: public nested types   =======================================
};

class File {
  public:
  // ======= BEGIN: public aliases ============================================

  typedef char                     ElementType;
  typedef std::vector<ElementType> FileBuffer;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public static methods =====================================

  static uint64_t s_getFileTimeStamp(const std::string& filename);

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public constructors =======================================

  File()
      : m_fp_(nullptr) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~File();

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  const char* getBuffer(size_t index = 0, size_t count = 0) const;

  bool getBuffer(FileBuffer&        buffer,
                 const std::string& startToken,
                 const std::string& endToken);

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool openFile(const std::string&  szFileName,
                FileType::Enum      fileType      = FileType::Binary,
                ReadWriteType::Enum readWriteType = ReadWriteType::Read);

  size_t readFileToBuffer(bool   appendToEndofBuffer = true,
                          size_t index               = 0,
                          size_t count               = 0);

  void closeFile();

  // TODO: seems not used
  bool isBufferEmpty() const { return m_buffer_.empty(); }

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc fields =======================================

  std::FILE*               m_fp_;
  std::vector<ElementType> m_buffer_;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FILE_H