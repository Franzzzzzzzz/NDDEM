EMSCRIPTEN_BINDINGS(my_class_example) {
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
        .function("getParticleForce", &Simulation<3>::getParticleForce)
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
        .function("getParticleForce", &Simulation<4>::getParticleForce)
        // .function("getX2", &Simulation<3>::getX2)
        .function("getBoundary", &Simulation<4>::getBoundary)
        .function("setBoundary", &Simulation<4>::setBoundary)
        .function("getWallForce", &Simulation<4>::getWallForce)
        .function("setExternalForce", &Simulation<4>::setExternalForce)
        .function("getTime", &Simulation<4>::getTime)
        ;
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
