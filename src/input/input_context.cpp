#include "input/input_context.h"

#include "utils/logger/global_logger.h"

namespace arise {

InputContextManager::InputContextManager() {
  m_contextStack.push(InputContext::Game);
  GlobalLogger::Log(LogLevel::Info, "InputContextManager initialized with Game context");
}

void InputContextManager::pushContext(InputContext context) {
  m_contextStack.push(context);

  std::string contextName = "";
  switch (context) {
    case InputContext::Game:
      contextName = "Game";
      break;
    case InputContext::UI:
      contextName = "UI";
      break;
    case InputContext::Menu:
      contextName = "Menu";
      break;
  }

  GlobalLogger::Log(LogLevel::Info, "Input context pushed: " + contextName);
}

void InputContextManager::popContext() {
  if (m_contextStack.size() > 1) { 
    m_contextStack.pop();
    GlobalLogger::Log(LogLevel::Info, "Input context popped");
  }
}

InputContext InputContextManager::getCurrentContext() const {
  if (m_contextStack.empty()) {
    GlobalLogger::Log(LogLevel::Error, "Input context stack is empty! This should never happen.");
    return InputContext::Game;
  }
  return m_contextStack.top();
}

bool InputContextManager::isContextActive(InputContext context) const {
  return getCurrentContext() == context;
}

}  // namespace arise