#include <Nous.h>

class Sandbox : public Nous::Application
{
public:
    Sandbox()
    {

    }

    ~Sandbox()
    {

    }
};

// 使用Nous的程序入口点，提供一个创建应用的函数
Nous::Application* Nous::CreateApplication()
{
    return new Sandbox();
}