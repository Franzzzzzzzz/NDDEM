#include <iostream>
#include "json_parser.h"

using json = nddem::json ; 

int main() {
    std::string input = R"({
        "name": "Alice",
        "age": 30,
        "grades": [-88.5, 91.2, -77],
        "active": true,
        "profile": {
            "id": 12345,
            "rank": "gold",
            "history": [[2020, 2021, 2022, 2023, 2024], [88,91.2,77,72.6,102]],
            "mapping": 
            {
              "alpha": "one", 
              "beta": "two", 
              "gamma": "three"
            }
        }
    })";

    try {
        json j ; 
        j=json::parse(input) ; 
        std::vector<double> gr = j["grades"].get<std::vector<double>>() ; fflush(stdout) ; 
        std::cout << j["profile"] ; 
        std::vector<std::vector<double>> history = j["profile"]["history"].get<std::vector<std::vector<double>>>() ;
        
        for (auto a:history)
        {
          printf("\n") ; 
          for (auto b : a)
            printf("%g ", b);
        }
        
        std::cout << "Parsed successfully! " << j["age"].get<int>() << "\n";
        //std::cout << j ; 
        
        for (auto & v: j["grades"])
          std::cout << v ; 
      
        for (auto v : j.items())
          std::cout << v.key() << " " << v.value() << "\n" ; 
        
        auto m = j["profile"]["mapping"].get<std::map<std::string,std::string>>() ; 
        
        std::cout << "=========================" ; 
        for (auto v:m)
          std::cout << v.first << "=>" << v.second << "\n" ; 
      
        
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
    }
    
    std::ifstream file("Mesh-D3.json");
    json j;
    file >> j;
    
    

}
