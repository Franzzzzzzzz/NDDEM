// #include <string>
// #include <vector>
// #include <map>
// #include <chrono>
// #include <stack>
//
//
//
// class Benchmark
// {
// public:
//     // Disabled versions:
//     static void start_clock ([[maybe_unused]]  const std::string &name ) {}
//     static void stop_clock ([[maybe_unused]]  const std::string &name ) {}
//     static void write_all() {} ;
//
//
// /*    static void start_clock (const std::string &name) {current.push(name) ; current_start.push(std::chrono::high_resolution_clock::now()) ;}
//     static void stop_clock (const std::string &name)
//     {
//         if (name!=current.top()) printf("Benchmark error: wrong stack %s\n", name.c_str()) ;
//         auto delay=std::chrono::high_resolution_clock::now() - current_start.top() ;
//         auto t=infos[name] ;
//         std::get<0>(t) ++ ;
//         std::get<1>(t) +=delay ;
//         infos[name]=t ;
//         current.pop() ; current_start.pop() ;
//     }
//     static void write_all()
//     {
//      printf("\n\nName       | Total      | Count      | Time/count |\n") ;
//      for (auto v:infos)
//      {
//       printf("%10s | %10g | %10d | %10g |\n", v.first.c_str(), std::get<1>(v.second).count(), std::get<0>(v.second), std::get<1>(v.second).count()/std::get<0>(v.second)) ;
//      }
//    }*/
//
// private:
//     static std::map <std::string, std::tuple<int, std::chrono::duration<double>>> infos ;
//     static std::stack<std::string> current ;
//     static std::stack<std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000> > >> current_start ;
// };
//
//
//
// std::map <std::string, std::tuple<int, std::chrono::duration<double>>> Benchmark::infos ;
// std::stack<std::string> Benchmark::current ;
// std::stack<std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000> > >> Benchmark::current_start ;
