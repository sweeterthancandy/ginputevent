#include "ginputevent/Emulation.h"

int main(){
        using namespace ginputevent;
        for(;;){
                StaticKeyboardCulture culture;
                ExecutionContext eCtx;
                eCtx.SleepBetweenMs(50);
                                                
                using namespace Frontend;
                std::vector<Variant> items;
                items.push_back( MouseRel(-10000, -10000));
                items.push_back( MouseRel(100, 100 ));
                items.push_back( Key("<BTN_LEFT>") );
                auto prog = Compile(culture, items);
                std::cout << "running\n";
                prog.Run(eCtx);
                sleep(1);
                return 0;
        }
}
