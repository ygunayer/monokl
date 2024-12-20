#include "action.h"

using namespace monokl;

ActionMapping::ActionMapping(SDL_KeyCode key, ActionType action)
  : key(key),
    flags(std::nullopt),
    action(action)
    {}

ActionMapping::ActionMapping(SDL_KeyCode key, uint16_t flags, ActionType action)
  : key(key),
    flags(flags),
    action(action)
    {}

bool ActionMapping::matches(const SDL_KeyboardEvent& event) const {
  if (event.keysym.sym != key) {
    return false;
  }

  if (!flags.has_value()) {
    return true;
  }

  uint16_t f = flags.value();

  return (f & event.keysym.mod) != 0;
}

OpenFilesAction::OpenFilesAction(const std::vector<std::string>& files)
  : Action(ActionType::OpenFiles),
    files(files)
    {}
