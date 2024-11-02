#include "pch.h"
#include "EditorEvent.h"

#include "Observer.h"

namespace Nous {

    std::vector<Observer*> EditorEventEmitter::observers;

    void EditorEventEmitter::AddObserver(Observer* observer)
    {
        observers.push_back(observer);
    }

    void EditorEventEmitter::Emit(EditorEvent& e)
    {
        for (auto& obs : observers)
        {
            obs->OnEditorEvent(e);
        }
    }
}