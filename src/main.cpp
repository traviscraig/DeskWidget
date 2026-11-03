#include <Arduino.h>
#include "Application.h"

namespace { dw::Application application; }

void setup() { application.setup(); }
void loop() { application.loop(); }