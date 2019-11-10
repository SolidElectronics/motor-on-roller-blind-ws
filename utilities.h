#include <EasyButton.h>

// Default button pin on NodeMCU
#define RESETBUTTON_PIN 0

namespace philsson {
namespace utilities {

class ResetButton
: public EasyButton
{
public:
  ResetButton();

  void setup(void (*callback)(void));
};

} // namespace philsson
} // namespace utilities