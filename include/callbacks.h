//
// Created by wlbf on 9/4/21.
//

#ifndef RIFT_CALLBACKS_H
#define RIFT_CALLBACKS_H

#include <functional>

namespace rift {
    using TimerCallback = std::function<void()>;
}

#endif //RIFT_CALLBACKS_H
