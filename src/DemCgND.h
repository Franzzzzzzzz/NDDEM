#include "Dem/DEMND.h"
#include "CoarseGraining/CoarseGraining.h"



class DEMCGND {
public:
    DEMCGND(int n) : S(n) {}
    Simulation<3> S ;
    CoarseGraining CG ;

    // function passing
    // Simulations
    void simu_finalise_init () {return S.finalise_init() ; }
    void simu_interpret_command (std::string s) {return S.interpret_command(s) ; }
    void simu_step_forward (int nt) {return S.step_forward(nt);}
    void simu_finalise() {return S.finalise() ; }
    std::vector<std::vector<double>> simu_getX() {return S.getX() ; }
    std::vector<double> simu_getRadii() {return S.getRadii() ; }
    void simu_fixParticle(int a, v1d loc) {return S.fixParticle(a, loc) ; }
    void simu_setFrozen(int a) {return S.setFrozen(a) ; }
    std::vector<std::vector<double>> simu_getOrientation() {return S.getOrientation() ; }
    std::vector<std::vector<double>> simu_getVelocity() {return S.getVelocity() ; }
    std::vector<std::vector<double>> simu_getParticleStress() {return S.getParticleStress() ; }
    std::vector<double> simu_getRotationRate() {return S.getRotationRate() ; }
    std::vector<double> simu_getBoundary(int a) {return S.getBoundary(a) ; }
    void simu_setBoundary(int a, std::vector<double> loc) {return S.setBoundary(a,loc);}
    std::vector<std::vector<double>> simu_getWallForce() {return S.getWallForce() ; }
    void simu_setExternalForce(int id, int duration, v1d force) {return S.setExternalForce(id,duration,force) ; }
    double simu_getTime() {return S.getTime() ; }

    //CoarseGraining
    void cg_setup_CG () {
        CG.setup_CG() ;
        reader = static_cast<InteractiveReader*> (CG.P.files[0].reader) ;
        reader->set_num_particles (S.N) ;
        reader->set_data (DataValue::radius, S.P.r) ;
        reader->set_data (DataValue::mass, S.P.m) ;
        reader->set_data (DataValue::Imom, S.P.I) ;
    }
    int cg_process_timestep (int ts, bool allow_avg_fluct=false) {return CG.process_timestep(ts, allow_avg_fluct) ; }
    std::vector<double> cg_get_result (int ts, std::string field, int component) {return CG.get_result(ts, field, component) ; }
    std::vector<double> cg_get_gridinfo () {return CG.get_gridinfo() ;}
    void cg_param_from_json_string(std::string param) {return CG.param_from_json_string(param) ;}
    std::vector<std::vector<double>> cg_param_get_bounds (int file=0) {return CG.param_get_bounds(file);}
    int cg_param_get_numts (int file = 0) {return CG.param_get_numts(file) ; }
    void cg_param_post_init() {return CG.param_post_init() ; }

    int cg_param_read_timestep ([[maybe_unused]] int n) {
        // In interactive mode, always read the current timestep
        for (auto & v:CG.C->CGP)
            for (auto & w:v.fields[0])
                w=0 ;
        reader->set_data(DataValue::pos, S.X) ;
        reader->set_data(DataValue::vel, S.V) ;

        reader->reset_contacts() ;
        for (auto & CLp : S.MP.CLp)
         for (auto & contact: CLp.v)
             if (contact.ghost==0)
                reader->add_contact({DataValue::id1, DataValue::id2, DataValue::fpq},
                                      {static_cast<double>(contact.i),static_cast<double>(contact.j),contact.infos->Fn[0]+contact.infos->Ft[0], contact.infos->Fn[1]+contact.infos->Ft[1], contact.infos->Fn[2]+contact.infos->Ft[2]}) ;

        reader->build_pospqlpq_from_ids (reader->data, 12, 13, 14, 17,
                                         reader->data, 3);



        return 0 ;
    }
private:
    InteractiveReader* reader ;

    std::vector<std::vector<double>> fpq ;
    std::vector<double> id1, id2 ;

} ;

#ifdef EMSCRIPTEN
    #include "em_bindings.h"
#endif
