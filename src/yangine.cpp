#include "Application.h"
#include "yangine.h"

namespace mynamespace
{
std::string helloWorld()
{
    Application app;

    app.Run(GetModuleHandle(NULL), 0);

    return 0;
}
} // namespace mynamespace