#ifndef SOLUTION_H_
#define SOLUTION_H_

#include "../util/util.h"

using namespace std;

class Request {
    public:
        int     id;
        double  twA;
        double  twB;
        double  coordX;
        double  coordY;
        double  demand;
        double  serviceTime;
        double  violationDemand;
        double  waitTimeStation = 0;
        double  rechargeRate;
        char    sigla[10];
        bool    batteryStation = false;

        Request(){}
        Request(int _id, double _twA, double _twB, double _coordX, double _coordY, double _demand, double _serviceTime, double _waitTimeStation, bool _batteryStation) 
        : id(_id), twA(_twA), twB(_twB), coordX(_coordX), coordY(_coordY), demand(_demand), serviceTime(_serviceTime), waitTimeStation(_waitTimeStation), batteryStation(_batteryStation) {}
};

class Data {

    public:
        int                             numTotalPoints;
        int                             numRequests;
        int                             numBatteryStations;
        int                             numVehicles;
        double                          betaTw = 345;
        double                          betaDemand = 425;
        double                          betaBattery = 460;
        double                          demandCapacity;
        double                          batteryCapacity;
        double                          rateConsumption;
        
        vector < Request >              requests;
        vector < Request >              orderRequests;
        vector < Request >              batteryStations;
        vector < vector < double > >    distances;
        vector < pair < pair < int, int >, double > >           closerStation;  // qual a estação mais próxima a cada cidade
        vector < pair < pair < double, double >, pair < pair < double, double >, int > > >  dataVehicles;
        //first is the demand and second is the battery and third is the consumption, fouth is the vehicle cost, and fifht is the amount of vehicle

        Data() {}

        Data(string file){
            ifstream fin(file.c_str());
            if(!fin){
                clog << "Deu ruim!" << endl;
                exit(0);
            }

            int     contClient = 0;
            int     contStation = 0;
            string  twA;
            string  twB;
            string  coordX;
            string  coordY;
            string  serviceTime;
            double  rechargeRate;
            
            string word1, word2, clientes, stations, type, stringId;

            //Head
            fin >> word1 >> word1 >> word1 >> word1 >> word1 >> word1 >> word1 >> word1;
            int k = 0;
            word1 = "recharging";

            //Data
            fin >> stringId >> type >> coordX >> coordY >> word2 >> twA >> twB >> serviceTime;
            while(stringId != "Q"){
                word1 = stringId;
                if(stringId.front() == 'S'){
                    requests.push_back(Request(k, stod(twA), stod(twB), stod(coordX), stod(coordY), stod(word2), stod(serviceTime), 0.0, true));
                    strcpy(requests.back().sigla, stringId.c_str());
                    contStation++;
                }else{
                    requests.push_back(Request(k, stod(twA), stod(twB), stod(coordX), stod(coordY), stod(word2), stod(serviceTime), 0.0, false));
                    strcpy(requests.back().sigla, stringId.c_str());
                    contClient++;
                }
                k++;
                fin >> stringId >> type >> coordX >> coordY >> word2 >> twA >> twB >> serviceTime;
            }

            batteryCapacity = stod(twA.substr(1,5));
            fin >> word1 >> word1 >> word1;
            demandCapacity = stod(word1.substr(1,4));
            fin >> word1 >> word1 >> word1 >> word1 >> word1;
            rateConsumption = stod(word1.substr(1,3));
            fin >> word1 >> word1 >> word1 >> word1 >> word1;
            rechargeRate = stod(word1.substr(1,4));

            numTotalPoints      = contStation + contClient;
            numRequests         = contClient;
            numBatteryStations  = contStation;

            for(int i=0; i < (int)requests.size(); i++){
                if(requests[i].batteryStation){
                    requests[i].rechargeRate = rechargeRate;
                }else{
                    requests[i].rechargeRate = 0;
                }
            }

            closerStation.resize(numTotalPoints);
            for(int i = 1; i < numTotalPoints; i++){
                closerStation[i].first.first =  requests[i].id;
                closerStation[i].first.second =  1;
                closerStation[i].second =  distances[i][1];
            }

            for(int i = 0; i < (int)closerStation.size(); i++){
                for(int j = 1; j < numTotalPoints; j++){
                    if(requests[j].batteryStation){
                        if(distances[closerStation[i].first.first][requests[j].id] < closerStation[i].second){
                            closerStation[i].second         = distances[closerStation[i].first.first][requests[j].id];
                            closerStation[i].first.second   = requests[j].id;
                        }
                    }
                }
            }
        }
};


class Vehicle{

    public:

        void intraRVND(                         Data &data);
        void intraSwap(                         Data &data);
        void showRoute();
        void intraShift2(                       Data &data);
        void evaluateVehicle(                   Data &data);
        void intraRealocation(                  Data &data);
        
        int                 numberStationsVisited   = 0;
        double              demand                  = 0;
        double              waitTw                  = 0;
        double              penality                = 0;
        double              batteryKm;
        double              batteryUsed             = 0;
        double              violationTw             = 0;
        double              vehicleCost;
        double              timeCharging            = 0;
        double              demandCapacity;
        double              violationDemand         = 0;
        double              violationBattery        = 0;
        double              batteryCapacity;
        double              rateConsumption;
        double              objective               = 0;
        double              ride                    = 0;
        double              justDistance            = 0;
        vector < Request >  route;
        vector < pair < int, double > > chargingTimeStation;
};

class Solution{
    public:
        void VNS(                           Data &data, int vnsMax);
        void shake(                         Data &data, int intensity, int &lastShake);
        void shakeSwap(                     Data &data);
        void interRVND(                     Data &data, int lastShake);
        void interSwap(                     Data &data);
        void interSwap2(                    Data &data);
        void addStation(                    Data &data);
        void interSwap2x1(                  Data &data);
        void removeStation(                 Data &data);
        void shakeRelocation(               Data &data);
        void interRelocation(               Data &data);
        void evaluateSolution(              Data &data);
        void interRelocation2(              Data &data);
        void shakeRemoveStation(            Data &data);
        void generateGreedySolution(        Data &data);
        void showSolution();
        
        int                 numberStationsVisited   = 0;
        int                 amountVehicles          = 0;
        double              demand                  = 0;
        double              waitTw                  = 0;
        double              penality                = 0;
        double              objective               = 0;
        double              violationTw             = 0;
        double              vehicleCost             = 0;
        double              justDistance            = 0;
        double              violationDemand         = 0;
        double              violationBattery        = 0;
        double              travelledDistance       = 0;
        double              ride                    = 0;
        vector < Vehicle >  vehicles;
};

#endif /* SOLUTION_H_ */

