#if defined(__APPLE__)

#pragma once

#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#include <MacTypes.h>

#include "platform.h"
#include "logging.h"
#include "event.h"
#include "window.h"

@interface MonoklMenuHandler : NSObject
-(instancetype)initWithApp:(monokl::Application*)app;
-(void)onNewWindow:(id)sender;
-(void)onOpen:(id)sender;
@end

namespace monokl {

struct MacPlatform : public Platform {
  MacPlatform();
  ~MacPlatform();

  void run_main_loop() override;
  void handle_event(std::shared_ptr<Event> event) override;
  PlatformType get_type() override;

  void setup_menu();

private:
  MonoklMenuHandler* menu_handler;
};

};

#endif
