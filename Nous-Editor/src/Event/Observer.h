#pragma once

#include "EditorEvent.h"

namespace Nous {

    class Observer
    {
    public:
        virtual void OnEditorEvent(EditorEvent& e) = 0;
    };

}

