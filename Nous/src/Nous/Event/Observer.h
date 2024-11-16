#pragma once

#include "AppEvent.h"

namespace Nous {

    class Observer
    {
    public:
        virtual void OnEditorEvent(AppEvent& e) = 0;
    };

}

