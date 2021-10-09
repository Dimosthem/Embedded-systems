#include <iostream>
#include "covidTrace.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <chrono>   
using namespace std;






int main(){
    //open all the files used
    serverFile.open("serverFile.bin");
    delayFile.open("delayFile.bin");
    //time when program starts
    
    
    unsigned int durationOfExperiment = 259200; //one tick is 0.1seconds, 7h and 12m is 259200 ticks
    float tick = 0.1*1000000; // 0.1sec or 10^5 usec
    unsigned int currentTick = 0;
    unsigned long currentDelay = 0; //current delay of the functions' calls  
    unsigned long previousDelay = 0;
    pthread_t BTNearCheckThread = 0;
    pthread_t COVIDTestThread = 0;
    auto start = std::chrono::high_resolution_clock::now(); 
    while (currentTick < durationOfExperiment){
       usleep(tick);
       if (currentTick>0){
          
           pthread_join(BTNearCheckThread,NULL);
          
           
           if(currentTick%1440 == 1 && currentTick!=1){
               
                
               pthread_join(COVIDTestThread, NULL);
               
           }
       }
       auto stop = std::chrono::high_resolution_clock::now();
       if (currentTick>0){
           auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start); 
           delayFile<<(duration.count()- 100000)<<endl;
       }
       start = std::chrono::high_resolution_clock::now();
       pthread_create(&BTNearCheckThread, NULL, BTChecking, NULL);  //the first line of BTChecking is the BTNearMe call, so I assume the call is of BTNearMe comes instantly after the call to BTChecking
       if (currentTick%1440 == 0 && currentTick !=0){               //one Covid test per 1440 ticks (tick = 0.1 sec)
           cout<<"Covid Test at Tick: "<<currentTick<<endl;
           pthread_create(&COVIDTestThread, NULL, testCOVIDandUpdateServer, &currentTick);
       }
       
       currentTick++;
    }
    
    serverFile.close();
    delayFile.close();
    cout<<"End of Program"<<endl;
    return 0;
}
