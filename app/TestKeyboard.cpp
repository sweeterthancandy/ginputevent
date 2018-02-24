#include "Emulation.h"

int main(){
        StaticKeyboardCulture culture;
        Frontend::Compiler cl(culture);
        ExecutionContext eCtx;
        eCtx.SleepBetweenMs(50);
                                        
        using namespace Frontend;
        std::vector<Variant> items;

        items.push_back( Key("a") );
        items.push_back( Key("A") );
        items.push_back( MakeVectorFromLiteral("abcdefghijklmnopqrstuvwxyz"));
        items.push_back( MakeVectorFromLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        items.push_back( MakeVectorFromLiteral("0123456789"));

        auto prog = cl.Compile(items);
        std::cout << "running\n";
        prog.Run(eCtx);
}
