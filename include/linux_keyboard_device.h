#ifndef LINUX_KEYBOARD_DEVICE_H
#define LINUX_KEYBOARD_DEVICE_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/input-event-codes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <boost/exception/all.hpp>

struct linux_keyboard_device{
        explicit linux_keyboard_device(const std::string& name)
        {
                
                fd_ = open("/dev/uinput", O_WRONLY | O_NONBLOCK );
                if( fd_ < 0 )
                        BOOST_THROW_EXCEPTION(std::domain_error("unable to open /dev/uinput"));

                struct uinput_setup usetup;



                /*
                 * The ioctls below will enable the device that is about to be
                 * created, to pass key events, in this case the space key.
                 */
                ioctl(fd_, UI_SET_EVBIT, EV_KEY);
                ioctl(fd_, UI_SET_EVBIT, EV_MSC);
                ioctl(fd_, UI_SET_EVBIT, EV_REL);

                for(int i=0;i!=KEY_MAX;++i){
                        ioctl(fd_, UI_SET_KEYBIT, i);
                }
                ioctl(fd_, UI_SET_MSCBIT, MSC_SCAN );
                        
                ioctl(fd_, UI_SET_RELBIT, REL_X);
                ioctl(fd_, UI_SET_RELBIT, REL_Y);

                memset(&usetup, 0, sizeof(usetup));
                usetup.id.bustype = BUS_USB;
                usetup.id.vendor = 0x1234; /* sample vendor */
                usetup.id.product = 0x5678; /* sample product */
                strcpy(usetup.name, "Example device");

                ioctl(fd_, UI_DEV_SETUP, &usetup);
                ioctl(fd_, UI_DEV_CREATE);

                /*
                 * On UI_DEV_CREATE the kernel will create the device node for this
                 * device. We are inserting a pause here so that userspace has time
                 * to detect, initialize the new device, and can start listening to
                 * the event, otherwise it will not notice the event we are about
                 * to send. This pause is only needed in our example code!
                 */
                sleep(1);


        }
        ~linux_keyboard_device(){
                ioctl(fd_, UI_DEV_DESTROY );
                close(fd_);
        }
        void emit(int type, int code, int value){
                struct input_event ie = {{0}};

                ie.type = type;
                ie.code = code;
                ie.value = value;
                /* timestamp values below are ignored */
                ie.time.tv_sec = 0;
                ie.time.tv_usec = 0;

                write(fd_, &ie, sizeof(ie));
        }
private:
        int fd_{-1};
};

#endif // LINUX_KEYBOARD_DEVICE_H
