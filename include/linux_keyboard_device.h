#ifndef LINUX_KEYBOARD_DEVICE_H
#define LINUX_KEYBOARD_DEVICE_H

#include <string>

struct linux_keyboard_device{
        explicit linux_keyboard_device(const std::string& name);
        ~linux_keyboard_device();
        void emit(int type, int code, int value);
private:
        int fd_{-1};
};

#endif // LINUX_KEYBOARD_DEVICE_H
