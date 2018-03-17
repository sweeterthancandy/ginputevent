#include "ginputevent/Emulation.h"
#include "ginputevent/linux_keyboard_device.h"
#include <thread>
#include <chrono>

namespace ginputevent{


ExecutionContext::ExecutionContext(){
        device_ = new linux_keyboard_device("linux-kbd");
}
void ExecutionContext::PostInstruction(){
        if( sleep_between_ != 0 ){
                std::this_thread::sleep_for( std::chrono::milliseconds(sleep_between_));
        }
}
} // end namespace ginputevent

