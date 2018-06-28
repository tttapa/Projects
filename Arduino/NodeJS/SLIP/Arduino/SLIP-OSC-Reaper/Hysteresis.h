#ifndef HYSTERESIS_H_
#define HYSTERESIS_H_

#include <stdint.h>

class Hysteresis
{
  public:
    uint8_t getOutputLevel(uint16_t inputLevel);

  private:
    uint8_t previousLevel = 0;
    const static uint8_t shiftFac = 3;
    const static uint8_t margin = (1 << shiftFac) - 1;
};

#endif // HYSTERESIS_H_
