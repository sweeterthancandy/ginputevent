#include "Emulation.h"
#include "linux_keyboard_device.h"

ExecutionContext::ExecutionContext(){
        device_ = new linux_keyboard_device("linux-kbd");
}
void ExecutionContext::PostInstruction(){
        if( sleep_between_ != 0 ){
                timespec ts = {0};
                timespec out = {0};
                ts.tv_sec = ( sleep_between_ / 1000 );
                ts.tv_nsec = ( sleep_between_ % 1000 ) * 1000000;
                nanosleep(&ts, &out);
        }
}
