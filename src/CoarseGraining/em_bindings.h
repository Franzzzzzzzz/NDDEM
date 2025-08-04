
EMSCRIPTEN_BINDINGS(CoarseGraining) {
  class_<CoarseGraining>("CoarseGraining")
    .constructor<>()
    .function("setup_CG", &CoarseGraining::setup_CG )  
    .function("process_timestep", &CoarseGraining::process_timestep)
    .function("process_fluct_from_avg", &CoarseGraining::process_fluct_from_avg)
    .function("process_all", &CoarseGraining::process_all)
    .function("get_result", &CoarseGraining::get_result)
    .function("get_spheres", &CoarseGraining::get_spheres)
    .function("get_gridinfo", &CoarseGraining::get_gridinfo)
    .function("write", &CoarseGraining::write)
    .function("param_from_json_string", &CoarseGraining::param_from_json_string)
    .function("param_get_bounds", &CoarseGraining::param_get_bounds)
    .function("param_get_numts", &CoarseGraining::param_get_numts)
    .function("param_get_minmaxradius", &CoarseGraining::param_get_minmaxradius)
    .function("param_read_timestep", &CoarseGraining::param_read_timestep)
    .function("param_post_init", &CoarseGraining::param_post_init)
    .function("debug", &CoarseGraining::debug)
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
