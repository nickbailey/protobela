#include "BelaContext.h"

// Native Bela API Emulation Layer
inline bool digitalRead(BelaContext* context, unsigned int frame, unsigned int pin) {
    return (context->digitalIn[frame] >> pin) & 0x1;
}

inline void digitalWrite(BelaContext* context, unsigned int frame, unsigned int pin, bool value) {
    if (value) {
        context->digitalOut[frame] |= (1 << pin);
    } else {
        context->digitalOut[frame] &= ~(1 << pin);
    }
}
