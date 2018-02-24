#include "Emulation.h"

int main(){
        for(;;){
                StaticKeyboardCulture culture;
                Frontend::Compiler cl(culture);
                ExecutionContext eCtx;
                eCtx.SleepBetweenMs(50);
                                                
                using namespace Frontend;
                std::vector<Variant> items;
                items.push_back( MouseRel(-10000, -10000));
                items.push_back( MouseRel(100, 100 ));
                items.push_back( Key("<BTN_LEFT>") );
                auto prog = cl.Compile(items);
                std::cout << "running\n";
                prog.Run(eCtx);
                sleep(1);
                return 0;
        }
}
