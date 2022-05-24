
EMSCRIPTEN_BINDINGS(DEMCGND) {
  class_<DEMCGND>("DEMCGND")
    .constructor<int>()
    .function("simu_finalise_init", &DEMCGND::simu_finalise_init)
    .function("simu_interpret_command", &DEMCGND::simu_interpret_command)
    .function("simu_step_forward", &DEMCGND::simu_step_forward)
    .function("simu_finalise", &DEMCGND::simu_finalise)
    .function("simu_getX", &DEMCGND::simu_getX)
    .function("simu_getRadii", &DEMCGND::simu_getRadii)
    .function("simu_fixParticle", &DEMCGND::simu_fixParticle)
    .function("simu_setFrozen", &DEMCGND::simu_setFrozen)
    .function("simu_getOrientation", &DEMCGND::simu_getOrientation)
    .function("simu_getVelocity", &DEMCGND::simu_getVelocity)
    .function("simu_getRotationRate", &DEMCGND::simu_getRotationRate)
    .function("simu_getParticleForce", &DEMCGND::simu_getParticleForce)
    .function("simu_getBoundary", &DEMCGND::simu_getBoundary)
    .function("simu_setBoundary", &DEMCGND::simu_setBoundary)
    .function("simu_getWallForce", &DEMCGND::simu_getWallForce)
    .function("simu_setExternalForce", &DEMCGND::simu_setExternalForce)
    .function("simu_getTime", &DEMCGND::simu_getTime)

    .function("cg_setup_CG", &DEMCGND::cg_setup_CG )
    .function("cg_process_timestep", &DEMCGND::cg_process_timestep)
    .function("cg_get_result", &DEMCGND::cg_get_result)
    .function("cg_get_gridinfo", &DEMCGND::cg_get_gridinfo)
    .function("cg_param_from_json_string", &DEMCGND::cg_param_from_json_string)
    .function("cg_param_get_bounds", &DEMCGND::cg_param_get_bounds)
    .function("cg_param_get_numts", &DEMCGND::cg_param_get_numts)
    .function("cg_param_read_timestep", &DEMCGND::cg_param_read_timestep)
    .function("cg_param_post_init", &DEMCGND::cg_param_post_init)
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
