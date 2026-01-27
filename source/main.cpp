#include "application.h"
#include "timer.h"

int main() {
    Timer::start();
    if (Application::init()) Timer::end(2, "Finished pre-initialization ");
    else {
        Timer::end(2, "Failed to initialize application ");
        return -1;
    }
    Timer::start();
    Application app;
    Timer::end(2, "Finished initialization ");
    app.mainloop();
    return 0;
}