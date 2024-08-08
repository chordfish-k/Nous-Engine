#include "pch.h"
#include "EditorEvent.h"

#include "Observer.h"

namespace Nous {

    std::vector<Observer*> EditorEventRepeater::observers;

    void EditorEventRepeater::AddObserver(Observer* observer)
    {
        observers.push_back(observer);
    }

    void EditorEventRepeater::Emit(EditorEvent& e)
    {
        for (auto& obs : observers)
        {
            obs->OnEditorEvent(e);
        }
    }
}