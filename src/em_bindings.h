EMSCRIPTEN_BINDINGS(DEMCG2D) {
  class_<DEMCG2D>("DEMCG2D")
    .constructor<int>()
    .function("simu_finalise_init", &DEMCG2D::simu_finalise_init)
    .function("simu_interpret_command", &DEMCG2D::simu_interpret_command)
    .function("simu_step_forward", &DEMCG2D::simu_step_forward)
    .function("simu_finalise", &DEMCG2D::simu_finalise)
    .function("simu_getX", &DEMCG2D::simu_getX)
    .function("simu_getRadii", &DEMCG2D::simu_getRadii)
    .function("simu_setRadius", &DEMCG2D::simu_setRadius)
    .function("simu_setMass", &DEMCG2D::simu_setMass)
    .function("simu_fixParticle", &DEMCG2D::simu_fixParticle)
    .function("simu_setFrozen", &DEMCG2D::simu_setFrozen)
    .function("simu_getOrientation", &DEMCG2D::simu_getOrientation)
    .function("simu_getVelocity", &DEMCG2D::simu_getVelocity)
    .function("simu_setVelocity", &DEMCG2D::simu_setVelocity)
    .function("simu_getRotationRate", &DEMCG2D::simu_getRotationRate)
    .function("simu_getContactForce", &DEMCG2D::simu_getContactForce)
    .function("simu_getContactInfos", &DEMCG2D::simu_getContactInfos)
    .function("simu_getBoundary", &DEMCG2D::simu_getBoundary)
    .function("simu_setBoundary", &DEMCG2D::simu_setBoundary)
    .function("simu_getWallForce", &DEMCG2D::simu_getWallForce)
    .function("simu_setExternalForce", &DEMCG2D::simu_setExternalForce)
    .function("simu_getTime", &DEMCG2D::simu_getTime)

    .function("cg_setup_CG", &DEMCG2D::cg_setup_CG )
    .function("cg_process_timestep", &DEMCG2D::cg_process_timestep)
    .function("cg_get_result", &DEMCG2D::cg_get_result)
    .function("cg_get_gridinfo", &DEMCG2D::cg_get_gridinfo)
    .function("cg_param_from_json_string", &DEMCG2D::cg_param_from_json_string)
    .function("cg_param_get_bounds", &DEMCG2D::cg_param_get_bounds)
    .function("cg_param_get_numts", &DEMCG2D::cg_param_get_numts)
    .function("cg_param_read_timestep", &DEMCG2D::cg_param_read_timestep)
    .function("cg_param_post_init", &DEMCG2D::cg_param_post_init)
    ;
}

EMSCRIPTEN_BINDINGS(DEMCG3D) {
  class_<DEMCG3D>("DEMCG3D")
    .constructor<int>()
    .function("simu_finalise_init", &DEMCG3D::simu_finalise_init)
    .function("simu_interpret_command", &DEMCG3D::simu_interpret_command)
    .function("simu_step_forward", &DEMCG3D::simu_step_forward)
    .function("simu_finalise", &DEMCG3D::simu_finalise)
    .function("simu_getX", &DEMCG3D::simu_getX)
    .function("simu_getRadii", &DEMCG3D::simu_getRadii)
    .function("simu_setRadius", &DEMCG3D::simu_setRadius)
    .function("simu_setMass", &DEMCG3D::simu_setMass)
    .function("simu_fixParticle", &DEMCG3D::simu_fixParticle)
    .function("simu_setFrozen", &DEMCG3D::simu_setFrozen)
    .function("simu_getOrientation", &DEMCG3D::simu_getOrientation)
    .function("simu_getVelocity", &DEMCG3D::simu_getVelocity)
    .function("simu_getRotationRate", &DEMCG3D::simu_getRotationRate)
    .function("simu_getContactForce", &DEMCG3D::simu_getContactForce)
    .function("simu_getContactInfos", &DEMCG3D::simu_getContactInfos)
    .function("simu_getBoundary", &DEMCG3D::simu_getBoundary)
    .function("simu_setBoundary", &DEMCG3D::simu_setBoundary)
    .function("simu_getWallForce", &DEMCG3D::simu_getWallForce)
    .function("simu_setExternalForce", &DEMCG3D::simu_setExternalForce)
    .function("simu_getTime", &DEMCG3D::simu_getTime)

    .function("cg_setup_CG", &DEMCG3D::cg_setup_CG )
    .function("cg_process_timestep", &DEMCG3D::cg_process_timestep)
    .function("cg_get_result", &DEMCG3D::cg_get_result)
    .function("cg_get_gridinfo", &DEMCG3D::cg_get_gridinfo)
    .function("cg_param_from_json_string", &DEMCG3D::cg_param_from_json_string)
    .function("cg_param_get_bounds", &DEMCG3D::cg_param_get_bounds)
    .function("cg_param_get_numts", &DEMCG3D::cg_param_get_numts)
    .function("cg_param_read_timestep", &DEMCG3D::cg_param_read_timestep)
    .function("cg_param_post_init", &DEMCG3D::cg_param_post_init)
    ;
}

EMSCRIPTEN_BINDINGS(DEMCG4D) {
  class_<DEMCG4D>("DEMCG4D")
    .constructor<int>()
    .function("simu_finalise_init", &DEMCG4D::simu_finalise_init)
    .function("simu_interpret_command", &DEMCG4D::simu_interpret_command)
    .function("simu_step_forward", &DEMCG4D::simu_step_forward)
    .function("simu_finalise", &DEMCG4D::simu_finalise)
    .function("simu_getX", &DEMCG4D::simu_getX)
    .function("simu_getRadii", &DEMCG4D::simu_getRadii)
    .function("simu_setRadius", &DEMCG4D::simu_setRadius)
    .function("simu_setMass", &DEMCG4D::simu_setMass)
    .function("simu_fixParticle", &DEMCG4D::simu_fixParticle)
    .function("simu_setFrozen", &DEMCG4D::simu_setFrozen)
    .function("simu_getOrientation", &DEMCG4D::simu_getOrientation)
    .function("simu_getVelocity", &DEMCG4D::simu_getVelocity)
    .function("simu_getRotationRate", &DEMCG4D::simu_getRotationRate)
    .function("simu_getContactForce", &DEMCG4D::simu_getContactForce)
    .function("simu_getContactInfos", &DEMCG4D::simu_getContactInfos)
    .function("simu_getBoundary", &DEMCG4D::simu_getBoundary)
    .function("simu_setBoundary", &DEMCG4D::simu_setBoundary)
    .function("simu_getWallForce", &DEMCG4D::simu_getWallForce)
    .function("simu_setExternalForce", &DEMCG4D::simu_setExternalForce)
    .function("simu_getTime", &DEMCG4D::simu_getTime)

    .function("cg_setup_CG", &DEMCG4D::cg_setup_CG )
    .function("cg_process_timestep", &DEMCG4D::cg_process_timestep)
    .function("cg_get_result", &DEMCG4D::cg_get_result)
    .function("cg_get_gridinfo", &DEMCG4D::cg_get_gridinfo)
    .function("cg_param_from_json_string", &DEMCG4D::cg_param_from_json_string)
    .function("cg_param_get_bounds", &DEMCG4D::cg_param_get_bounds)
    .function("cg_param_get_numts", &DEMCG4D::cg_param_get_numts)
    .function("cg_param_read_timestep", &DEMCG4D::cg_param_read_timestep)
    .function("cg_param_post_init", &DEMCG4D::cg_param_post_init)
    ;
}

EMSCRIPTEN_BINDINGS(DEMCG5D) {
  class_<DEMCG5D>("DEMCG5D")
    .constructor<int>()
    .function("simu_finalise_init", &DEMCG5D::simu_finalise_init)
    .function("simu_interpret_command", &DEMCG5D::simu_interpret_command)
    .function("simu_step_forward", &DEMCG5D::simu_step_forward)
    .function("simu_finalise", &DEMCG5D::simu_finalise)
    .function("simu_getX", &DEMCG5D::simu_getX)
    .function("simu_getRadii", &DEMCG5D::simu_getRadii)
    .function("simu_setRadius", &DEMCG5D::simu_setRadius)
    .function("simu_setMass", &DEMCG5D::simu_setMass)
    .function("simu_fixParticle", &DEMCG5D::simu_fixParticle)
    .function("simu_setFrozen", &DEMCG5D::simu_setFrozen)
    .function("simu_getOrientation", &DEMCG5D::simu_getOrientation)
    .function("simu_getVelocity", &DEMCG5D::simu_getVelocity)
    .function("simu_getRotationRate", &DEMCG5D::simu_getRotationRate)
    .function("simu_getContactForce", &DEMCG5D::simu_getContactForce)
    .function("simu_getContactInfos", &DEMCG5D::simu_getContactInfos)
    .function("simu_getBoundary", &DEMCG5D::simu_getBoundary)
    .function("simu_setBoundary", &DEMCG5D::simu_setBoundary)
    .function("simu_getWallForce", &DEMCG5D::simu_getWallForce)
    .function("simu_setExternalForce", &DEMCG5D::simu_setExternalForce)
    .function("simu_getTime", &DEMCG5D::simu_getTime)

    .function("cg_setup_CG", &DEMCG5D::cg_setup_CG )
    .function("cg_process_timestep", &DEMCG5D::cg_process_timestep)
    .function("cg_get_result", &DEMCG5D::cg_get_result)
    .function("cg_get_gridinfo", &DEMCG5D::cg_get_gridinfo)
    .function("cg_param_from_json_string", &DEMCG5D::cg_param_from_json_string)
    .function("cg_param_get_bounds", &DEMCG5D::cg_param_get_bounds)
    .function("cg_param_get_numts", &DEMCG5D::cg_param_get_numts)
    .function("cg_param_read_timestep", &DEMCG5D::cg_param_read_timestep)
    .function("cg_param_post_init", &DEMCG5D::cg_param_post_init)
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
