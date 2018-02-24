#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <linux/input.h>
#include <linux/uinput.h>
#include <boost/variant.hpp>
#include <regex>

#include "linux_keyboard_device.h"
#include "KeyboardCulture.h"


struct ExecutionContext{
        ExecutionContext(){
                device_ = new linux_keyboard_device("linux-kbd");
        }
        linux_keyboard_device* Device(){ return device_; }
        void PostInstruction(){
                if( sleep_between_ != 0 ){
                        timespec ts = {0};
                        timespec out = {0};
                        ts.tv_sec = ( sleep_between_ / 1000 );
                        ts.tv_nsec = ( sleep_between_ % 1000 ) * 1000000;
                        nanosleep(&ts, &out);
                }
        }
        void SleepBetweenMs(unsigned ms){
                sleep_between_ = ms;
        }
private:
        linux_keyboard_device* device_;
        unsigned sleep_between_{0};
};

struct Instruction{
        virtual void Execute(ExecutionContext& ctx)const=0;
};

struct MouseAbsInstr : Instruction{
        explicit MouseAbsInstr(int x, int y):x_{x}, y_{y}{}
        void Execute(ExecutionContext& ctx)const override{
                ctx.Device()->emit( EV_ABS, ABS_X, x_);
                ctx.Device()->emit( EV_ABS, ABS_Y, y_);
        }
private:
        int x_;
        int y_;
};

struct MouseRelInsr : Instruction{
        explicit MouseRelInsr(int x, int y):x_{x}, y_{y}{}
        void Execute(ExecutionContext& ctx)const override{
                ctx.Device()->emit( EV_REL, REL_X, x_);
                ctx.Device()->emit( EV_REL, REL_Y, y_);
        }
private:
        int x_;
        int y_;
};

struct LinuxKeyDownInstr : Instruction{
        explicit LinuxKeyDownInstr(KeyDecl const* key):key_{key}{}
        void Execute(ExecutionContext& ctx)const override{
                //std::cerr << "Pressing down " << *key_<< "\n";
                ctx.Device()->emit( EV_MSC, MSC_SCAN, key_->LinuxKey()  );
                ctx.Device()->emit( EV_KEY, key_->LinuxKey(), 1 );
        }
private:
        KeyDecl const* key_;
};

struct LinuxKeyUpInstr : Instruction{
        explicit LinuxKeyUpInstr(KeyDecl const* key):key_{key}{}
        void Execute(ExecutionContext& ctx)const override{
                //std::cerr << "Pressing up " << *key_ << "\n";
                ctx.Device()->emit( EV_MSC, MSC_SCAN, key_->LinuxKey()  );
                ctx.Device()->emit( EV_KEY, key_->LinuxKey(), 0 );
        }
private:
        KeyDecl const* key_;
};

struct TimeoutInstrMs : Instruction{
        explicit TimeoutInstrMs(unsigned ms){
                //ts_.tv_nsec = nano_sec;
                #if 0
                ts_.tv_sec  = ms / 1000;
                ts_.tv_nsec = 1000000 * ( ms % 1000 );
                #endif
                ts_.tv_sec  = 1;
        }
        void Execute(ExecutionContext&)const override{
                timespec out = {0};
                nanosleep(&ts_, &out);
        }
private:
        struct timespec ts_ = {0};

};

struct CompositeInstr : Instruction{
        void Push(Instruction* instr){
                vec_.push_back(instr);
        }
        void Execute(ExecutionContext& ctx)const override{
                for( auto const& _ : vec_){
                        _->Execute(ctx);
                }

        }
private:
        std::vector<Instruction*> vec_;
};

struct CtrlSyncInstr : Instruction{
        void Execute(ExecutionContext& ctx)const override{
                ctx.Device()->emit( EV_SYN, SYN_REPORT, 0 );
        }

};


struct Program{
        void Push(Instruction* instr){
                vec_.push_back(instr);
        }
        void Run(ExecutionContext& ctx)const{
                for( auto const& _ : vec_){
                        _->Execute(ctx);
                        ctx.PostInstruction();
                }
        }
private:
        std::vector<Instruction*> vec_;
};



namespace Frontend{

        struct Key{
                Key(std::string const& c):c_{c}{}
                auto const& Get()const{ return c_; }
        private:
                std::string c_;
        };

        struct MouseRel{
                MouseRel(int x_, int y_):x{x_}, y{y_}{}
                int x;
                int y;
        };
        
        struct MouseAbs{
                MouseAbs(int x_, int y_):x{x_}, y{y_}{}
                int x;
                int y;
        };

        enum VectorType{
                VectorType_Nested,
                VectorType_Single,
        };


        struct Vector{
                explicit Vector(VectorType type = VectorType_Single):type_{type}{}
                void Push(Key const& lit){
                        vec_.push_back(lit);
                }
                auto begin()const{ return vec_.begin(); }
                auto end()const{ return vec_.end(); }
                auto GetType()const{ return type_; }
        private:
                VectorType type_;
                std::vector<Key> vec_;
        };

        struct SleepMs{
                SleepMs(unsigned ms):ms_{ms}{}
                auto Get()const{ return ms_; }
        private:
                unsigned ms_;
        };

        template<class... Args>
        Vector MakeNestedVector(Args&&... args){
                Vector vec(VectorType_Nested);
                int aux[] = { ( vec.Push(Key{args}), 0)... };
                return vec;
        }
        template<class... Args>
        Vector MakeVector(Args&&... args){
                Vector vec(VectorType_Single);
                int aux[] = { ( vec.Push(Key{args}), 0)... };
                return vec;
        }
        Vector MakeVectorFromLiteral(std::string const& lit){
                Vector vec(VectorType_Single);
                for( char c : lit ){
                        vec.Push(Key{std::string{c}});
                }
                return vec;
        }

        using Variant = boost::variant< Key, MouseRel, MouseAbs, Vector, SleepMs >;


        struct CompilerImpl{
                explicit CompilerImpl(KeyboardCulture const& culture)
                        :culture_{&culture}
                {
                        shift_down_ = new LinuxKeyDownInstr( culture.Map("<RIGHTSHIFT>") );
                        shift_up_   = new LinuxKeyUpInstr( culture.Map("<RIGHTSHIFT>") );
                        sync_       = new CtrlSyncInstr{};
                        _25ms_      = new TimeoutInstrMs{25};
                }
                KeyboardCulture const* culture_;
                LinuxKeyDownInstr* shift_down_;
                LinuxKeyUpInstr*   shift_up_;
                CtrlSyncInstr*     sync_;
                TimeoutInstrMs* _25ms_;
        };

        struct CompilerDevice : boost::static_visitor<>
        {
                explicit CompilerDevice(CompilerImpl const* self):self_{self}{}
                void operator()(Key const& key)const{
                        KeyDecl const* decl = self_->culture_->Map(key.Get());
                        if( decl == nullptr ){
                                std::cerr << "Can't find mapping for " << key.Get() << "\n";
                                return;
                        }
                        auto comp = new CompositeInstr;

                        if( decl->IsUpper() ){
                                comp->Push( self_->shift_down_ );
                        }
                        comp->Push( new LinuxKeyDownInstr( decl ) );

                        comp->Push( self_->sync_ );

                        comp->Push( new LinuxKeyUpInstr( decl ) );
                        
                        if( decl->IsUpper() ){
                                comp->Push( self_->shift_up_ );
                        }

                        comp->Push( self_->sync_ );

                        prog.Push(comp);

                }
                void operator()(MouseRel const& mouserel)const{
                        auto comp = new CompositeInstr;

                        comp->Push( new MouseRelInsr(mouserel.x, mouserel.y ));
                        comp->Push( self_->sync_ );

                        prog.Push(comp);
                }
                void operator()(MouseAbs const& mouseabs)const{
                        auto comp = new CompositeInstr;

                        comp->Push( new MouseAbsInstr(mouseabs.x, mouseabs.y ));
                        comp->Push( self_->sync_ );

                        prog.Push(comp);
                }
                void operator()(Vector const& vec)const{
                        

                        switch(vec.GetType()){
                        case VectorType_Nested:{
                                auto comp = new CompositeInstr;
                                for( auto const& _ : vec ){
                                        KeyDecl const* decl = self_->culture_->Map(_.Get());

                                        if( decl->IsUpper() ){
                                                comp->Push( self_->shift_down_ );
                                        }
                                        comp->Push( new LinuxKeyDownInstr( decl ) );

                                        comp->Push( self_->sync_ );

                                }

                                auto iter = vec.begin();
                                auto end = vec.end();
                                for(;iter!=end;){
                                        --end;

                                        KeyDecl const* decl = self_->culture_->Map(end->Get());

                                        if( decl->IsUpper() ){
                                                comp->Push( self_->shift_up_ );
                                        }
                                        comp->Push( new LinuxKeyUpInstr( decl ) );

                                        comp->Push( self_->sync_ );
                                }
                                prog.Push(comp);
                                break;
                        }
                        case VectorType_Single:{
                                for( auto const& _ : vec ){
                                        (*this)(_);
                                }
                                break;
                        }
                        }
                                
                }
                void operator()(SleepMs const& sms)const{
                        prog.Push( new TimeoutInstrMs(sms.Get()) );
                }
                mutable Program prog;
                CompilerImpl const* self_;

        };

        struct Compiler : CompilerImpl{
                explicit Compiler(KeyboardCulture const& culture):CompilerImpl{culture}{}
                Program Compile(std::vector<Frontend::Variant> const& vec)const{
                        CompilerDevice impl(this);

                        std::for_each( vec.begin(), vec.end(), boost::apply_visitor( impl ) );

                        return std::move(impl.prog);
                }
        private:

        };

} // Frontend


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

