#ifndef GAME_ENGINE_WINDOW_H
#define GAME_ENGINE_WINDOW_H

#include "event/event.h"

#include <SDL.h>
#include <math_library/dimension.h>
#include <math_library/point.h>

#include <string>
#include <utility>

namespace game_engine {

class Window {
  public:
  enum class Flags {
    None              = 0,
    Fullscreen        = SDL_WINDOW_FULLSCREEN,
    Shown             = SDL_WINDOW_SHOWN,
    Borderless        = SDL_WINDOW_BORDERLESS,
    Resizable         = SDL_WINDOW_RESIZABLE,
    AllowHighDpi      = SDL_WINDOW_ALLOW_HIGHDPI,
    FullscreenDesktop = SDL_WINDOW_FULLSCREEN_DESKTOP,
    Vulkan            = SDL_WINDOW_VULKAN
  };

  // YAGNI
  Window()
      : m_title_("")
      , m_size_(0, 0)
      , m_position_(0, 0)
      , m_flags_(Flags::None) {}

  Window(std::string        title,
         math::Dimension2Di size,
         math::Point2Di     position,
         Flags              flags)
      : m_title_{std::move(title)}
      , m_size_{std::move(size)}
      , m_position_{std::move(position)}
      , m_flags_{flags}
      , m_window_{SDL_CreateWindow(m_title_.c_str(),
                                   m_position_.x(),
                                   m_position_.y(),
                                   m_size_.width(),
                                   m_size_.height(),
                                   static_cast<Uint32>(m_flags_))} {}

  Window(const Window&)                    = delete;
  auto operator=(const Window&) -> Window& = delete;
  Window(Window&&)                         = delete;
  auto operator=(Window&&) -> Window&      = delete;

  ~Window() {
    if (m_window_) {
      SDL_DestroyWindow(m_window_);
      m_window_ = nullptr;
    }
  }

  // clang-format off

  [[nodiscard]] auto getTitle() const -> const std::string& { return m_title_; }
  [[nodiscard]] auto getSize() const -> const math::Dimension2Di& { return m_size_; }
  [[nodiscard]] auto getPosition() const -> const math::Point2Di& { return m_position_; }
  [[nodiscard]] auto getFlags() const -> Flags { return m_flags_; }

  // clang-format on

  private:
  std::string        m_title_;
  math::Dimension2Di m_size_;
  math::Point2Di     m_position_;
  Flags              m_flags_;
  SDL_Window*        m_window_{nullptr};
};

}  // namespace game_engine

#endif  // GAME_ENGINE_WINDOW