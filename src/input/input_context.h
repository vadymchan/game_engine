#ifndef ARISE_INPUT_CONTEXT_H
#define ARISE_INPUT_CONTEXT_H

#include <stack>

namespace arise {

enum class InputContext {
  Game,
  UI,
  Menu
};

class InputContextManager {
  public:
  InputContextManager();
  void         pushContext(InputContext context);
  void         popContext();
  InputContext getCurrentContext() const;
  bool         isContextActive(InputContext context) const;

  private:
  std::stack<InputContext> m_contextStack;
};

}  // namespace arise

#endif  // ARISE_INPUT_CONTEXT_H