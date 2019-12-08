#include "ginputevent/EmulationFrontend.h"
#include "ginputevent/EmulationInstructions.h"
#include <iostream>

namespace ginputevent{
namespace Frontend{
namespace{
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
                                std::cerr << "Can't find mapping for " << key << "\n";
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
                Program Run(std::vector<Frontend::Variant> const& vec)const{
                        CompilerDevice impl(this);

                        std::for_each( vec.begin(), vec.end(), boost::apply_visitor( impl ) );

                        return std::move(impl.prog);
                }
        };
} // anon
} // Frontend

namespace Frontend{


        Program Compile(KeyboardCulture const& culture, std::vector<Frontend::Variant> const& vec){
                Compiler cc(culture);
                return cc.Run(vec);
        }

} // Frontend
} // end namespace ginputevent

