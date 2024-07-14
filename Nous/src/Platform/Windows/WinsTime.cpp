#include "pch.h"
#include "WinsTime.h"

namespace Nous {

    Time* Time::m_Instance = new WinsTime();

}