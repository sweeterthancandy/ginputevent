#ifndef INCLUDE_EMULATIONINSTRUCTION_H
#define INCLUDE_EMULATIONINSTRUCTION_H

#include "linux_keyboard_device.h"

#include <thread>
#include <chrono>

namespace ginputevent{


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
        explicit TimeoutInstrMs(unsigned ms):ms_{ms}{}
        void Execute(ExecutionContext&)const override{
                std::this_thread::sleep_for(std::chrono::milliseconds(ms_));
        }
private:
        unsigned ms_;
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

} // end namespace ginputevent



#endif // INCLUDE_EMULATIONINSTRUCTION_H
