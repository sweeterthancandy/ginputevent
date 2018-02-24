#include "Emulation.h"

int main(){
        StaticKeyboardCulture culture;
        ExecutionContext eCtx;
        eCtx.SleepBetweenMs(50);
                                        
        using namespace Frontend;
        std::vector<Variant> items;

        items.push_back( Key("a") );
        items.push_back( Key("A") );
        items.push_back( MakeVectorFromLiteral("abcdefghijklmnopqrstuvwxyz"));
        items.push_back( MakeVectorFromLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        items.push_back( MakeVectorFromLiteral("0123456789"));

        auto prog = Compile(culture, items);
        std::cout << "running\n";
        prog.Run(eCtx);
}

#if 0
struct TestDriver{
        void Run(){
                std::vector<std::string> username_vec{
                };
                std::vector<std::string> password_vec{
                };

                std::vector<std::string> email_vec{
                };

                size_t count = username_vec.size() * 
                               password_vec.size() * 
                               email_vec.size();
                size_t done = 0;
                
                StaticKeyboardCulture culture;
                Frontend::Compiler cl(culture);
                ExecutionContext eCtx;
                eCtx.SleepBetweenMs(50);
                

                for(auto const& username : username_vec ){
                        for(auto const& password : password_vec ){
                                for(auto const& email : email_vec ){
                                        #if 0
                                        std::stringstream token;
                                        token 
                                                << "{" << username << "}"
                                                << "{" << password << "}"
                                                << "{" << email << "}";
                                        #endif
                                        using namespace Frontend;
                                        std::vector<Variant> items;
                                        if( done == 0 ){
                                                items.push_back( MakeNestedVector( "<LEFTALT>", "<TAB>") );
                                        }
                                        items.push_back( MakeNestedVector( "<LEFTCTRL>", "t") );
                                        items.push_back( Key("<ENTER>") );
                                        items.push_back( SleepMs(1000) );

                                        #if 0
                                        // first put the combination in the url
                                        items.push_back( MakeNestedVector( "<LEFTCTRL>", "l") );
                                        items.push_back( MakeVectorFromLiteral( token.str() ));
                                        items.push_back( Key("<ESC>") );
                                        #endif

                                        items.push_back( MakeVector("<TAB>", "<TAB>", "<TAB>" ) );
                                        items.push_back( MakeVectorFromLiteral( username) );
                                        items.push_back( Key("<TAB>") );
                                        items.push_back( MakeVectorFromLiteral( password) );
                                        items.push_back( Key("<TAB>") );
                                        items.push_back( MakeVectorFromLiteral( email ) );
                                        items.push_back( Key("<TAB>") );
                                        items.push_back( MakeVectorFromLiteral( "united kingdom") );
                                        items.push_back( MakeVector("<TAB>", "<TAB>", "<TAB>") );
                                        items.push_back( Key("<SPACE>") );
                                        items.push_back( Key("<TAB>") );
                                        items.push_back( Key("<ENTER>") );

                                        auto prog = cl.Compile(items);

                                        std::cout 
                                                << "Trying\n"
                                                << "    username=" << username << "\n"
                                                << "    password=" << password << "\n"
                                                << "    email="    << email << "\n"
                                        ;

                                        prog.Run(eCtx);

                                        ++done;
                                        double pct = ( done * 100.0 / count );
                                        std::cout << "Done " << done << " out of " << count << " ( " << pct << "% )\n";
                                        sleep(2);
                                }
                        }
                }
        }
};
#endif
