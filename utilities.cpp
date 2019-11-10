#include "utilities.h"

namespace philsson {
namespace utilities {

ResetButton::ResetButton()
: EasyButton(RESETBUTTON_PIN)
{}

void ResetButton::setup(void (*callback)(void))
{
  if (!callback)
  {
    Serial.println("Button Callback is nullptr");
    return;
  }

  // Initialize the button.
  begin();

  // Add the callback function
  onPressedFor(3000 /*ms*/, callback);
}

} // namespace philsson
} // namespace utilities