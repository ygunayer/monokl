#if defined(__APPLE__)

#include "platforms/macos/macos_platform.h"
#include "platform.h"

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
    m_app->get_event_bus()->publish(std::make_shared<Event>(
      id.value(),
      std::make_shared<Action>(ActionType::OpenNewWindow)
    ));
  } else {
    WindowOptions options;
    m_app->create_window(options);
  }
}

-(void)onOpen:(id)sender {
  log_debug("Open");
}
@end

MacPlatform::MacPlatform() {
  std::setlocale(LC_ALL, "C.UTF-8");

  action_mappings = {
    ActionMapping(SDLK_f, KMOD_SHIFT, ActionType::ToggleFavoritesOnly),

    ActionMapping(SDLK_f, KMOD_GUI, ActionType::MaximizeWindow),
    ActionMapping(SDLK_m, KMOD_GUI, ActionType::MinimizeWindow),
    // ActionMapping(SDLK_n, KMOD_GUI, ActionType::OpenNewWindow),

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
  this->app->get_event_bus()->subscribe([this](std::shared_ptr<Event> event) {
    this->handle_event(event);
  });

  WindowOptions options;
  this->app->create_window(options);

  this->menu_handler = [[MonoklMenuHandler alloc] initWithApp:this->app.get()];
}

MacPlatform::~MacPlatform() {
  if (app != nullptr) {
    app.reset();
  }
}

PlatformType MacPlatform::get_type() {
  return PlatformType::MacOS;
}

void MacPlatform::handle_event(std::shared_ptr<Event> event) {
}

void MacPlatform::setup_menu() {
  NSApplication *app = [NSApplication sharedApplication];
  NSMenu *menu = [app mainMenu];
  // log_debug("Menu: %s", ((char*)[menu.title UTF8String]));

  // File
  NSMenuItem* fileMenuItem = [NSMenuItem new];
  [menu insertItem:fileMenuItem atIndex:1];

  NSMenu* fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
  [fileMenuItem setSubmenu:fileMenu];

  // File > New Window
  NSMenuItem* newWindowItem = [fileMenu addItemWithTitle:@"New Window" action:@selector(onNewWindow:) keyEquivalent:@"n"];
  [newWindowItem setTarget:this->menu_handler];

  // --
  NSMenuItem* sep1 = [NSMenuItem separatorItem];
  [fileMenu addItem:sep1];

  // File > Open
  NSMenuItem* openMenuItem = [fileMenu addItemWithTitle:@"Open" action:@selector(onOpen:) keyEquivalent:@"o"];
  [newWindowItem setTarget:this->menu_handler];

  // File > Open Recent
  NSMenuItem* openRecentMenuItem = [fileMenu addItemWithTitle:@"Open Recent" action:nil keyEquivalent:@""];
  [newWindowItem setTarget:this->menu_handler];

  // File > Open Recent > ...
  NSMenu* openRecentMenu = [[NSMenu alloc] initWithTitle:@"Open Recent"];
  [openRecentMenuItem setSubmenu:openRecentMenu];

  log_debug("Set up menu");
}

void MacPlatform::run_main_loop() {
  setup_menu();

  log_debug("Running main loop");
  this->app->run_main_loop();
}

#endif
