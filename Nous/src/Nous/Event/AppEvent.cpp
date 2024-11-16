#include "pch.h"
#include "AppEvent.h"

#include "Observer.h"

namespace Nous {

    std::vector<Observer*> AppEventEmitter::observers;

    void AppEventEmitter::AddObserver(Observer* observer)
    {
        observers.push_back(observer);
    }

    void AppEventEmitter::Emit(AppEvent& e)
    {
        for (auto& obs : observers)
        {
            obs->OnEditorEvent(e);
        }
    }
}