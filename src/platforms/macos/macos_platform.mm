#include "platform.h"
#include "event.h"
#include "window.h"
#include "platforms/macos/macos_platform.h"

using namespace monokl;

@implementation MonoklMenuHandler {
  Application* m_app;
}

-(instancetype)initWithApp:(Application*)app {
  self = [super init];
  if (self) {
    self->m_app = app;
  }
  return self;
}

-(void)onNewWindow:(id)sender {
  auto id = m_app->get_last_window_id();
  if (id.has_value()) {
    m_app->get_event_bus()->publish(Event(id.value(), Action(ActionType::OpenNewWindow)));
  } else {
    WindowOptions options;
    m_app->create_window(options);
  }
}
@end

MacPlatform::MacPlatform() {
  std::setlocale(LC_ALL, "C.UTF-8");

  action_mappings = {
    ActionMapping(SDLK_f, KMOD_SHIFT, ActionType::ToggleFavoritesOnly),

    ActionMapping(SDLK_f, KMOD_GUI, ActionType::MaximizeWindow),
    ActionMapping(SDLK_m, KMOD_GUI, ActionType::MinimizeWindow),
    ActionMapping(SDLK_n, KMOD_GUI, ActionType::OpenNewWindow),

    ActionMapping(SDLK_LEFT, ActionType::GoToPrevious),
    ActionMapping(SDLK_RIGHT, ActionType::GoToNext),
    ActionMapping(SDLK_HOME, ActionType::GoToFirst),
    ActionMapping(SDLK_END, ActionType::GoToLast),
    ActionMapping(SDLK_KP_0, ActionType::FitImageToScreen),
    ActionMapping(SDLK_KP_1, ActionType::ResetZoom),
    ActionMapping(SDLK_f, ActionType::ToggleFavorite),
  };

  user_home_dir = std::filesystem::path(std::getenv("HOME"));

  window_flags = SDL_WINDOW_METAL;

  ApplicationSettings settings = ApplicationSettings::load();
  settings.action_mappings = action_mappings;
  this->app = std::make_unique<Application>(settings);

  WindowOptions options;
  this->app->create_window(options);

  this->menu_handler = [[MonoklMenuHandler alloc] initWithApp:this->app.get()];
}

MacPlatform::~MacPlatform() {
  if (app != nullptr) {
    app.reset();
  }
}

void MacPlatform::run_main_loop() {
  NSApplication *app = [NSApplication sharedApplication];
  NSMenu *menu = [app mainMenu];
  // log_debug("Menu: %s", ((char*)[menu.title UTF8String]));

  NSMenuItem* appMenuItem = [NSMenuItem new];
  NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@"File"];
  NSMenuItem* newWindowItem = [appMenu addItemWithTitle:@"New Window" action:@selector(onNewWindow:) keyEquivalent:@"n"];
  [newWindowItem setTarget:this->menu_handler];
  [appMenuItem setSubmenu:appMenu];
  [menu insertItem:appMenuItem atIndex:1];

  const char* pool_debug = [[NSAutoreleasePool debugDescription] UTF8String];
  log_debug("Autorelease pool: %s", pool_debug);

  log_debug("Running main loop");
  this->app->run_main_loop();
}
