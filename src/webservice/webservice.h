#import "stlvector.h"




//struct mons__Validate
//{
//    unsigned int year ;
//    unsigned int month ;
//    unsigned int day ;
//    unsigned int hour ;
//    unsigned int min ;
//    unsigned int sec ;

//};


//int mons__AddInspectModule(std::string id,string OrderNO,string AdName,int CinemaNum,mons__Validate start,
//                           mons__Validate end,int ShowOder,int Type,std::string ModulePath );


int mons__AddInspectModule(std::string id,std::string OrderNO,std::string AdName,int CinemaNum,std::string start,
                           std::string end,int ShowOder,int Type,std::string ModulePath,int &ret );
