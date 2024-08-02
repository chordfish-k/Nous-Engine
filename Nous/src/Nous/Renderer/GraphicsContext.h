#pragma once

namespace Nous {

    /**
     * 图形上下文，接口，分离窗口和渲染API
     */
    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;

        /**
         * 图形API初始化
         */
        virtual void Init() = 0;
        /**
         * 交换缓冲区，绘制到屏幕
         */
        virtual void SwapBuffers() = 0;
    };

}
