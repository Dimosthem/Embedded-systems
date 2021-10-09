#include <iostream>
#include <pthread.h>
#include <sys/time.h>
#include <fstream>
#include <vector>
using namespace std;
//total number of acquaintances is 30.
//We assume that the chance of meeting an acquaintance is 10% per tick.
const int NUM_OF_ACQ = 30;
const int PROB_OF_MEETING = 100; // it means 10% of meeting someone per tick  
const int PROB_OF_POSITIVE = 50; // 5% probability of being found positive to COVID

typedef int macaddress; // assume the value is 48bit, since, there is not a 48bit primitive type.
//all the created files
ofstream serverFile;
ofstream delayFile;

//the macAddress and how many ticks ago that macAddress was discovered 
typedef struct myPair{
    macaddress mcAddr;
    unsigned int timeTicks;
}myPair;


vector <myPair> foundMacAddresses;
vector <macaddress> MacAddressesIndices(NUM_OF_ACQ, -1);

pthread_mutex_t contacts_mutex;
vector <myPair> contacts; 



//remove certain item from myPair vector 
void remove(vector<myPair>& vec, size_t pos)
{
    vector<myPair>::iterator it = vec.begin();
    advance(it, pos);
    vec.erase(it);
}
//remove certain address from contacts 
void findAndRemoveContact(macaddress mc){
    
    for (int i=0; i<contacts.size(); i++){
        if (contacts[i].mcAddr == mc){
            remove(contacts, i);
        }
    }
    
}
//in case of deletion of an item from foundMacAddresses, correct indices in MacAddressesIndices so that they point to the correct cell in foundMacAddresses
void correctIndices(int position){
    for (int i = position +1; i< foundMacAddresses.size(); i++){
        MacAddressesIndices[foundMacAddresses[i].mcAddr]--;
    }
}

int randomIntGenerator(int range){
    srand((unsigned) time(0));
    return rand() % range;
}
macaddress BTnearMe(){
    
    if (randomIntGenerator(1000)<PROB_OF_MEETING){
        int personMet = randomIntGenerator(NUM_OF_ACQ); //a random person out of the # acquaintances
        return personMet;
    }
    else{
        return -1; //-1 means nobody was met
    }
    
}

void* BTChecking (void* args){
    
    
    macaddress personMet = BTnearMe();
    

    // +1 to all timeTicks of found macAddresses and remove macAddresses older than 120ticks (20mins)
    for (int i =0; i<foundMacAddresses.size(); i++){
        foundMacAddresses[i].timeTicks +=1;
        if (foundMacAddresses[i].timeTicks >120){
            correctIndices(i);
            MacAddressesIndices[foundMacAddresses[i].mcAddr] = -1;
            remove(foundMacAddresses, i);
        }
    }
    pthread_mutex_lock (&contacts_mutex);
    // +1 to all timeTicks of contacts and remove contacts older than 120960ticks (14 days)
    for (int i =0; i<contacts.size(); i++){
        contacts[i].timeTicks +=1;
        if (contacts[i].timeTicks> 120960){
            remove(contacts, i);
        }
    }
    pthread_mutex_unlock (&contacts_mutex);
    //if met a person
    if(personMet !=-1){
        
        int index = MacAddressesIndices[personMet];
        //if person exists in foundMacAddresses and was met again 24-120 ticks ago
        if (index >-1 && foundMacAddresses[index].timeTicks >= 24 && foundMacAddresses[index].timeTicks <= 120){
            myPair personAndTime;
            personAndTime.mcAddr = personMet;
            personAndTime.timeTicks = 0;
            pthread_mutex_lock (&contacts_mutex);
            findAndRemoveContact(personMet);    //renew timer in contacts (in case person already existed there)
            contacts.push_back(personAndTime);
            pthread_mutex_unlock (&contacts_mutex);
            foundMacAddresses[index].timeTicks = 0; //renew timer in foundMacAddresses
         
        }
        //if that person didnt exist in foundMacAddresses
        else if(index == -1) {
            
            myPair personAndTime;
            personAndTime.mcAddr = personMet;
            personAndTime.timeTicks = 0;
            foundMacAddresses.push_back(personAndTime);
            MacAddressesIndices[personMet] = foundMacAddresses.size() - 1;
        }
        
    }

}

bool testCOVID(){
    int result = randomIntGenerator(1000);
    cout<<"the result in testCovid is "<<(result < PROB_OF_POSITIVE)<<endl;
    return result < PROB_OF_POSITIVE; 
}

void uploadContacts(int currentTick){
    serverFile<<"Time of upload: "<<currentTick<<" tick"<<endl;
    for (int i=0; i<contacts.size(); i++){
        
        serverFile<<contacts[i].mcAddr<<endl;
    }
    contacts.clear();
}

void* testCOVIDandUpdateServer(void* args){
    int* noisyTick = ((int*)args); //it's noisy because no locks were used in order to not delay the next function call even more 
    int currentTick = (*noisyTick)/1440 * 1440; //it should be a multiple of 1440
    
    if(testCOVID()){
        pthread_mutex_lock (&contacts_mutex);
        cout<<"Server Upload at "<<currentTick<<endl;
        cout<<"Size of uploaded contacts "<<contacts.size()<<endl;
        uploadContacts(currentTick);
        pthread_mutex_unlock (&contacts_mutex);
    }
}