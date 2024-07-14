#pragma once

#include "Nous/Core/Timestep.h"

#include "GLFW/glfw3.h"

namespace Nous {

    class WinsTime : public Time
    {
    public:
        Timestep GetNowTime() { return (float) glfwGetTime(); }
    };

}
