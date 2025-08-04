#include "Preprocessor_macros.h"

#define EMFUNCTION(val,name) function( #name, &Simulation<val>::name)
#define EMMACRO(r,state) EMMACRO2(BOOST_PP_TUPLE_ELEM(2, 0, state))
#define EMMACRO2(dim) \
    class_<Simulation<dim>>("Simulation" XSTR(dim))\
        .constructor<int>()\
        .EMFUNCTION(dim,finalise_init)\
        .EMFUNCTION(dim,interpret_command)\
        .EMFUNCTION(dim,step_forward)\
        .EMFUNCTION(dim,finalise)\
        .EMFUNCTION(dim,getX)\
        .EMFUNCTION(dim,getRadii)\
        .EMFUNCTION(dim,setRadius)\
        .EMFUNCTION(dim,setMass)\
        .EMFUNCTION(dim,fixParticle)\
        .EMFUNCTION(dim,setFrozen)\
        .EMFUNCTION(dim,getOrientation)\
        .EMFUNCTION(dim,getVelocity)\
        .EMFUNCTION(dim,setVelocity)\
        .EMFUNCTION(dim,getRotationRate)\
        .EMFUNCTION(dim,setAngularVelocity)\
        .EMFUNCTION(dim,getContactForce)\
        .EMFUNCTION(dim,getContactInfos)\
        .EMFUNCTION(dim,getBoundary)\
        .EMFUNCTION(dim,setBoundary)\
        .EMFUNCTION(dim,getWallForce)\
        .EMFUNCTION(dim,setExternalForce)\
        .EMFUNCTION(dim,getTime)\
        .EMFUNCTION(dim,randomDrop)\
        ;

EMSCRIPTEN_BINDINGS(my_class_example) {
    BOOST_PP_FOR((1, MAXDIM), PRED, OP, EMMACRO)
    /*class_<Simulation<2>>("Simulation2")
        .constructor<int>()
        .function("finalise_init", &Simulation<2>::finalise_init)
        .function("interpret_command", &Simulation<2>::interpret_command)
        .function("step_forward", &Simulation<2>::step_forward)
        .function("finalise", &Simulation<2>::finalise)
        // .smart_ptr<std::shared_ptr<Simulation<3>>>("Simulation")
        // .property("X", &Simulation<3>::getX, &Simulation<3>::setX)
        .function("getX", &Simulation<2>::getX)
        .function("getRadii", &Simulation<2>::getRadii)
        .function("setRadius", &Simulation<2>::setRadius)
        .function("setMass", &Simulation<2>::setMass)
        .function("fixParticle", &Simulation<2>::fixParticle)
        .function("setFrozen", &Simulation<2>::setFrozen)
        .function("getOrientation", &Simulation<2>::getOrientation)
        .function("getVelocity", &Simulation<2>::getVelocity)
        .function("setVelocity", &Simulation<2>::setVelocity)
        .function("getRotationRate", &Simulation<2>::getRotationRate)
        .function("getContactForce", &Simulation<2>::getContactForce)
        .function("getContactInfos", &Simulation<2>::getContactInfos)
        // .function("getX2", &Simulation<3>::getX2)
        .function("getBoundary", &Simulation<2>::getBoundary)
        .function("setBoundary", &Simulation<2>::setBoundary)
        .function("getWallForce", &Simulation<2>::getWallForce)
        .function("setExternalForce", &Simulation<2>::setExternalForce)
        .function("getTime", &Simulation<2>::getTime)
        ;
    class_<Simulation<3>>("Simulation3")
        .constructor<int>()
        .function("finalise_init", &Simulation<3>::finalise_init)
        .function("interpret_command", &Simulation<3>::interpret_command)
        .function("step_forward", &Simulation<3>::step_forward)
        .function("finalise", &Simulation<3>::finalise)
        // .smart_ptr<std::shared_ptr<Simulation<3>>>("Simulation")
        // .property("X", &Simulation<3>::getX, &Simulation<3>::setX)
        .function("getX", &Simulation<3>::getX)
        .function("getRadii", &Simulation<3>::getRadii)
        .function("setRadius", &Simulation<3>::setRadius)
        .function("setMass", &Simulation<3>::setMass)
        .function("fixParticle", &Simulation<3>::fixParticle)
        .function("setFrozen", &Simulation<3>::setFrozen)
        .function("getOrientation", &Simulation<3>::getOrientation)
        .function("getVelocity", &Simulation<3>::getVelocity)
        .function("getRotationRate", &Simulation<3>::getRotationRate)
        .function("getContactForce", &Simulation<3>::getContactForce)
        .function("getContactInfos", &Simulation<3>::getContactInfos)
        // .function("getX2", &Simulation<3>::getX2)
        .function("getBoundary", &Simulation<3>::getBoundary)
        .function("setBoundary", &Simulation<3>::setBoundary)
        .function("getWallForce", &Simulation<3>::getWallForce)
        .function("setExternalForce", &Simulation<3>::setExternalForce)
        .function("getTime", &Simulation<3>::getTime)
        ;
    class_<Simulation<4>>("Simulation4")
        .constructor<int>()
        .function("finalise_init", &Simulation<4>::finalise_init)
        .function("interpret_command", &Simulation<4>::interpret_command)
        .function("step_forward", &Simulation<4>::step_forward)
        .function("finalise", &Simulation<4>::finalise)
        // .smart_ptr<std::shared_ptr<Simulation<3>>>("Simulation")
        // .property("X", &Simulation<3>::getX, &Simulation<3>::setX)
        .function("getX", &Simulation<4>::getX)
        .function("getRadii", &Simulation<4>::getRadii)
        .function("setRadius", &Simulation<4>::setRadius)
        .function("setMass", &Simulation<4>::setMass)
        .function("fixParticle", &Simulation<4>::fixParticle)
        .function("setFrozen", &Simulation<4>::setFrozen)
        .function("getOrientation", &Simulation<4>::getOrientation)
        .function("getVelocity", &Simulation<4>::getVelocity)
        .function("getRotationRate", &Simulation<4>::getRotationRate)
        .function("getContactForce", &Simulation<4>::getContactForce)
        .function("getContactInfos", &Simulation<4>::getContactInfos)
        // .function("getX2", &Simulation<3>::getX2)
        .function("getBoundary", &Simulation<4>::getBoundary)
        .function("setBoundary", &Simulation<4>::setBoundary)
        .function("getWallForce", &Simulation<4>::getWallForce)
        .function("setExternalForce", &Simulation<4>::setExternalForce)
        .function("getTime", &Simulation<4>::getTime)
        ;
    class_<Simulation<5>>("Simulation5")
        .constructor<int>()
        .function("finalise_init", &Simulation<5>::finalise_init)
        .function("interpret_command", &Simulation<5>::interpret_command)
        .function("step_forward", &Simulation<5>::step_forward)
        .function("finalise", &Simulation<5>::finalise)
        // .smart_ptr<std::shared_ptr<Simulation<3>>>("Simulation")
        // .property("X", &Simulation<3>::getX, &Simulation<3>::setX)
        .function("getX", &Simulation<5>::getX)
        .function("getRadii", &Simulation<5>::getRadii)
        .function("setRadius", &Simulation<5>::setRadius)
        .function("setMass", &Simulation<5>::setMass)
        .function("fixParticle", &Simulation<5>::fixParticle)
        .function("setFrozen", &Simulation<5>::setFrozen)
        .function("getOrientation", &Simulation<5>::getOrientation)
        .function("getVelocity", &Simulation<5>::getVelocity)
        .function("getRotationRate", &Simulation<5>::getRotationRate)
        .function("getContactForce", &Simulation<5>::getContactForce)
        .function("getContactInfos", &Simulation<5>::getContactInfos)
        // .function("getX2", &Simulation<3>::getX2)
        .function("getBoundary", &Simulation<5>::getBoundary)
        .function("setBoundary", &Simulation<5>::setBoundary)
        .function("getWallForce", &Simulation<5>::getWallForce)
        .function("setExternalForce", &Simulation<5>::setExternalForce)
        .function("getTime", &Simulation<5>::getTime)
        ;*/
        
}

// EMSCRIPTEN_BINDINGS(stl_wrappers) {
//     emscripten::register_vector<double>("Vec1DDouble");
//     emscripten::register_vector<std::vector<double>>("Vec2DDouble");
// }

namespace emscripten {
namespace internal {

template <typename T, typename Allocator>
struct BindingType<std::vector<T, Allocator>> {
    using ValBinding = BindingType<val>;
    using WireType = ValBinding::WireType;

    static WireType toWireType(const std::vector<T, Allocator> &vec) {
        return ValBinding::toWireType(val::array(vec));
    }

    static std::vector<T, Allocator> fromWireType(WireType value) {
        return vecFromJSArray<T>(ValBinding::fromWireType(value));
    }
};

template <typename T>
struct TypeID<T,
              typename std::enable_if_t<std::is_same<
                  typename Canonicalized<T>::type,
                  std::vector<typename Canonicalized<T>::type::value_type,
                              typename Canonicalized<T>::type::allocator_type>>::value>> {
    static constexpr TYPEID get() { return TypeID<val>::get(); }
};

}  // namespace internal
}  // namespace emscripten
