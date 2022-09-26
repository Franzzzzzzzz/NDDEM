


// enum DataValue {radius, mass, Imom, pos, vel, omega, id1, id2, pospq, lpq, fpq, mpq, mqp} ;
//                     0    1     2    3    6    9      12   13   14     17   20   23   26

class InteractiveReader : public Reader {
public:
    InteractiveReader() {data.resize(29) ; }
    
    int get_numts () {return ts ; }
    int get_num_particles() {return N ; }
    void set_num_particles (int NN) {
        N=NN ; 
        for (int i=0 ; i<12 ; i++) data[i].resize(N) ; 
    }
    int get_num_contacts () {return Nc;}
    void set_num_contacts(int NN) {
        Nc = NN ; 
        for (int i=12 ; i<29 ; i++) data[i].resize(Nc) ; 
    }
    
    double * get_data(DataValue datavalue, int dd, std::string name="") 
    {
     switch (datavalue)
     {
        case DataValue::radius : return &(data[0][0]) ;
        case DataValue::mass : return &(data[1][0] ) ;
        case DataValue::Imom : return &(data[2][0] ) ;
        
        case DataValue::pos : return &(data[3+dd][0] ) ;
        case DataValue::vel : return &(data[6+dd][0] ) ;
        case DataValue::omega : return &(data[9+dd][0] ) ;
        
        case DataValue::id1 : return &(data[12][0]) ;
        case DataValue::id2 : return &(data[13][0] ) ;
        
        case DataValue::pospq : return &(data[14+dd][0] ) ;
        case DataValue::lpq   : return &(data[17+dd][0] ) ;
        case DataValue::fpq   : return &(data[20+dd][0] ) ;
        case DataValue::mpq   : return &(data[23+dd][0] ) ;
        case DataValue::mqp   : return &(data[26+dd][0] ) ;
        default: return nullptr ; 
     }
    }
    
    void set_data (DataValue destination, std::vector<std::vector<double>> & orig)
    {
     int length=0, idx=0 ; 
     switch (destination)
     {
         case DataValue::pos : length = N ; idx=3 ; break ; 
         case DataValue::vel : length = N ; idx=6 ; break ;
         case DataValue::omega: length = N; idx=9 ; break ; 
         default: printf("Unknown value kind (InteractiveReader::set_data v2d)\n") ;         
     }
     for (int i=0 ; i<length ; i++)
        for (int dd=0 ; dd<3 ; dd++)
            data[idx+dd][i]=orig[i][dd] ; 
    }
    
    void set_data (DataValue destination, std::vector<double> & orig)
    {
     int length=0, idx=0 ; 
     switch (destination)
     {
         case DataValue::radius : length = N ; idx=0 ; break ; 
         case DataValue::mass : length = N ; idx=1 ; break ;
         case DataValue::Imom: length = N; idx=2 ; break ; 
         case DataValue::id1: length = Nc; idx=12 ; break ; 
         case DataValue::id2: length = Nc; idx=13 ; break ; 
         default: printf("Unknown value kind (InteractiveReader::set_data v1d)\n") ;         
     }
     for (int i=0 ; i<length ; i++)
        data[idx][i]=orig[i] ; 
    }
    
    void add_contact(std::vector<DataValue> dv, std::vector<double> value)
    {
        if (Nc>=maxNc)
        {
            maxNc += growth ; 
            for (int i=12; i<29 ; i++)
                data[i].resize(maxNc) ;
        }
        
        int id=0 ; 
        for (auto d: dv)
        {
            switch(d) {
                case DataValue::id1: data[12][Nc]=value[id] ; id++; break ; 
                case DataValue::id2: data[13][Nc]=value[id] ; id++; break ;
                case DataValue::pospq: for (int i=0 ; i<3 ; i++, id++) data[14+i][Nc]=value[id] ; break ;
                case DataValue::lpq:   for (int i=0 ; i<3 ; i++, id++) data[17+i][Nc]=value[id] ; break ;
                case DataValue::fpq:   for (int i=0 ; i<3 ; i++, id++) data[20+i][Nc]=value[id] ; break ;
                case DataValue::mpq:   for (int i=0 ; i<3 ; i++, id++) data[23+i][Nc]=value[id] ; break ;
                case DataValue::mqp:   for (int i=0 ; i<3 ; i++, id++) data[26+i][Nc]=value[id] ; break ;
                default: printf("Unknown Datavalue (InteractiveReader::add_contact)\n") ; 
            }
        }
        Nc++ ; 
    }
    
    void reset_contacts ()
    {
        Nc= 0 ; 
    }
    
    v2d data ;
    
private :
    int ts=0 ;
    int N=0 ; 
    int Nc=0 ; 
    int maxNc=0 ; 
    const int growth=100 ; 
} ; 
