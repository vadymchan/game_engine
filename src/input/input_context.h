#ifndef GAME_ENGINE_INPUT_CONTEXT_H
#define GAME_ENGINE_INPUT_CONTEXT_H

#include <stack>

namespace game_engine {

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

}  // namespace game_engine

#endif  // GAME_ENGINE_INPUT_CONTEXT_H