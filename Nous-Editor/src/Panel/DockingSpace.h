#pragma once

namespace Nous {

    class DockingSpace
    {
    public:
        static void BeginDocking(bool hasMenuBar = true);
        static void EndDocking();
    };

}

