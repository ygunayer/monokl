#pragma once

#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>

#include "platform.h"



@interface MonoklMenuHandler : NSObject
-(instancetype)initWithApp:(monokl::Application*)app;
-(void)onNewWindow:(id)sender;
@end

namespace monokl {

struct MacPlatform : public Platform {
  MacPlatform();
  ~MacPlatform();

  void run_main_loop() override;

private:
  MonoklMenuHandler* menu_handler;
};

};
