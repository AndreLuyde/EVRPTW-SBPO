#include "solution.h"

void Solution::VNS(Data &data, int vnsMax) {
    Solution bestSolution, currentSolution, validSolution, smallVehiclesSolution;
    smallVehiclesSolution.amountVehicles = -1;
    int iter = 0, bestNumVehicles = 99, lastShake = 0;
    double bestObjective;

    generateGreedySolution(data);
    evaluateSolution(data);

    cout << "Inicial Solution" << endl;
    showSolution();
    
    bestObjective = objective;
    currentSolution = *this;
    validSolution = *this;
    
    while (iter < vnsMax) {
        shake(data, iter, lastShake);
    
        interRVND(data, lastShake);
    
        if(violationDemand < EPS && violationBattery < EPS && violationTw < EPS){
    
            if(validSolution.objective - objective > EPS){
                validSolution = *this;
            }
    
            if(amountVehicles < bestNumVehicles){
                bestNumVehicles = amountVehicles;
                smallVehiclesSolution = *this;
    
            }
        }
    
        if(bestObjective - objective > EPS ){
            showSolution();
            bestObjective   = objective;
            currentSolution = *this;
            iter            = 0;
        }else{
            iter++;
            *this           = currentSolution;
        }
    }
    
    if(violationDemand > EPS || violationBattery > EPS || violationTw > EPS){
        *this = validSolution;
    }

    if(smallVehiclesSolution.amountVehicles != -1 && smallVehiclesSolution.amountVehicles < amountVehicles){
        *this = smallVehiclesSolution;
    }
    
    vector < pair < int, int > > idRemove;
    Vehicle car;
    car.route.push_back(data.requests[0]);
    car.route.push_back(data.requests[0]);
    bool onliBat = true;
    for(int i = 0; i < (int)vehicles.size(); i++){
        vehicles[i].vehicleCost = 0;
        if(vehicles[i].route.size() > 2){
            onliBat = true;
            for(int j = 0; j < (int)vehicles[i].route.size(); j++){
                if(!vehicles[i].route[j].batteryStation){
                    onliBat = false;
                    break;
                }
            }
            if(onliBat){
                vehicles[i].route = car.route;
            }
        }
    }
    evaluateSolution(data);
}

void Solution::generateGreedySolution(Data &data) {
    int k = data.numBatteryStations;
    double batterConsumption, wait = 0, vTw = 0;

    if(data.numRequests >= 100){
        vehicles.resize(20);                                          //quantidade de veiculos    
        amountVehicles = 20;
    }else if(data.numRequests >= 15){
        vehicles.resize(10);                                          //quantidade de veiculos    
        amountVehicles = 10;
    }else if(data.numRequests >= 10){
        vehicles.resize(10);                                          //quantidade de veiculos    
        amountVehicles = 10;
    }else{
        vehicles.resize(10);                                          //quantidade de veiculos    
        amountVehicles = 10;
    }
    
    for (int i = 0; i < (int)vehicles.size(); i++){
        vehicles[i].demandCapacity          = data.demandCapacity;       //limite de demanda
        vehicles[i].batteryCapacity         = data.batteryCapacity;      //limite de bateria
        vehicles[i].rateConsumption         = data.rateConsumption;      //consumo de bateria por KM
        vehicles[i].vehicleCost             = 1000;                  //custo do Veículo
        vehicles[i].batteryKm               = data.batteryCapacity / vehicles[i].rateConsumption;
        vehicles[i].batteryUsed             = 0;
        vehicles[i].waitTw                  = 0;
        vehicles[i].demand                  = 0;
        vehicles[i].violationTw             = 0;
        vehicles[i].violationBattery        = 0;
        vehicles[i].violationDemand         = 0;
        vehicles[i].violationBattery        = 0;
        vehicles[i].objective               = 0;
        vehicles[i].justDistance            = 0;
        vehicles[i].numberStationsVisited   = 0;
        vehicles[i].penality                = 0;
        vehicles[i].ride                    = 0;

        vehicles[i].route.push_back(data.requests[0]);                              //iniciar com deposito
    }

    int i = 0;
    while(k < (int)data.requests.size()){
        if(!data.requests[k].batteryStation){
            if(vehicles[i].demand < vehicles[i].demandCapacity){
                batterConsumption = data.distances[vehicles[i].route.back().id][data.requests[k].id]; // Consumo de bateria até o cliente
                if(vehicles[i].batteryUsed + batterConsumption > vehicles[i].batteryKm){
                    vehicles[i].violationBattery = (vehicles[i].batteryUsed + batterConsumption) - vehicles[i].batteryKm;
                }

                if((vehicles[i].batteryUsed + batterConsumption > vehicles[i].batteryKm)){
                    if (!vehicles[i].route.back().batteryStation){
                        vehicles[i].ride                    += data.closerStation[vehicles[i].route.back().id].second + data.requests[data.closerStation[vehicles[i].route.back().id].first.second].waitTimeStation;
                        vehicles[i].justDistance            += data.closerStation[vehicles[i].route.back().id].second;
                        vehicles[i].batteryUsed             += data.closerStation[vehicles[i].route.back().id].second;
                        vehicles[i].numberStationsVisited++;
                        vehicles[i].route.push_back(data.requests[data.closerStation[vehicles[i].route.back().id].first.second]);
                        vehicles[i].timeCharging = ((vehicles[i].batteryUsed * vehicles[i].rateConsumption)) * vehicles[i].route.back().rechargeRate;
                        vehicles[i].chargingTimeStation.push_back(make_pair(data.requests[data.closerStation[vehicles[i].route.back().id].first.second].id, vehicles[i].timeCharging));
                        vehicles[i].batteryUsed = 0;
                    }
                }

                if(vehicles[i].demand + data.requests[k].demand <= vehicles[i].demandCapacity){
                    vehicles[i].ride                += data.distances[vehicles[i].route.back().id][data.requests[k].id];
                    vehicles[i].justDistance       += data.distances[vehicles[i].route.back().id][data.requests[k].id];
                    wait = 0, vTw = 0;

                    if(vehicles[i].ride < data.requests[k].twA){
                        wait                        = data.requests[k].twA - vehicles[i].ride;
                        vehicles[i].ride  = data.requests[k].twA;
                    }else if(vehicles[i].ride > data.requests[k].twB){
                        vTw                         = vehicles[i].ride - data.requests[k].twB;
                        vehicles[i].ride  = data.requests[k].twB;
                    }

                    vehicles[i].ride                += data.requests[k].serviceTime;
                    vehicles[i].batteryUsed         += data.distances[vehicles[i].route.back().id][data.requests[k].id];
                    vehicles[i].route.push_back(data.requests[k]);
                    vehicles[i].demand              += data.requests[k].demand;
                    vehicles[i].waitTw              += wait;
                    vehicles[i].violationTw         += vTw;
                    k++;
                }else{
                    i++;
                }
            }else{
                if(i == (int)vehicles.size()){
                    cout << "NUMERO DE VEICULOS EXEDIDO" << endl;
                    exit(1);
                }
                i++;
            }
        }else{
            k++;
        }
    }

    for (int i = 0; i < (int)vehicles.size(); i++){
        if(vehicles[i].route.size() < 2){
            vehicles[i].route.push_back(data.requests[0]);                              //finalizar veículos vazios
        }
    }

    for (i = 0; i < (int)vehicles.size(); i++){
        if(vehicles[i].route.back().id != 0){
            vehicles[i].penality = (vehicles[i].violationTw * data.betaTw) + (vehicles[i].violationDemand * data.betaDemand) + (vehicles[i].violationBattery * data.betaBattery);
            vehicles[i].ride   += data.distances[vehicles[i].route.back().id][0];
            vehicles[i].justDistance        += data.distances[vehicles[i].route.back().id][0];
            vehicles[i].batteryUsed         += data.distances[vehicles[i].route.back().id][0];
            vehicles[i].route.push_back(data.requests[0]);                          //volta pro deposito com deposito
        }

        numberStationsVisited           += vehicles[i].numberStationsVisited;
        ride               += vehicles[i].ride;
        justDistance                    += vehicles[i].justDistance;
        demand                          += vehicles[i].demand;
        penality                        += vehicles[i].penality;

        if(vehicles[i].route.size() > 2){
            vehicleCost                    += vehicles[i].vehicleCost;
        }
        vehicles[i].objective += vehicles[i].penality + vehicles[i].vehicleCost + vehicles[i].justDistance;
    }
    objective = justDistance + vehicleCost + penality;
}

void Solution::evaluateSolution(Data &data){
    waitTw                  = 0;
    violationTw             = 0;
    violationDemand         = 0;
    violationBattery        = 0;
    justDistance            = 0;
    numberStationsVisited   = 0;
    penality                = 0;
    vehicleCost             = 0;
    ride                    = 0;
    amountVehicles          = 0;

    for (int i = 0; i < (int)vehicles.size(); i++) {
        vehicles[i].evaluateVehicle(data);
        violationTw             += vehicles[i].violationTw;
        violationBattery        += vehicles[i].violationBattery;
        violationDemand         += vehicles[i].violationDemand;
        justDistance            += vehicles[i].justDistance;
        numberStationsVisited   += vehicles[i].numberStationsVisited;
        penality                += vehicles[i].penality;
        ride                    += vehicles[i].ride;

        if((int)vehicles[i].route.size() > 2){
            vehicleCost                    += vehicles[i].vehicleCost;
            amountVehicles++;
        }
    }
    objective = justDistance + vehicleCost + penality;
}

void Vehicle::evaluateVehicle(Data &data){
    ride = 0; 
    demand = 0;
    penality = 0;
    violationTw = 0;
    batteryUsed = 0;
    justDistance = 0;
    numberStationsVisited = 0;
    violationBattery = 0;
    violationDemand = 0;

    if(route.size() > 2){
        for(int i = 0; i < (int)route.size()-1; i++){

            if(route[i].batteryStation){                              
                numberStationsVisited++;
                if(batteryKm - batteryUsed < EPS){
                    timeCharging        = (batteryKm * rateConsumption) * route[i].rechargeRate;
                    batteryUsed         = 0; //carga total
                }else{
                    timeCharging        = (batteryUsed * rateConsumption) * route[i].rechargeRate;
                    batteryUsed         = 0; //carga suficiente
                }
                ride += timeCharging;
            }else{
                if(ride < route[i].twA){                                //janela A
                    ride = route[i].twA;
                }else if(ride > route[i].twB){                          //janela B
                    violationTw += (ride - route[i].twB);
                    ride = route[i].twB;
                }
    
                ride += route[i].serviceTime;                           //tempo de serviço
                demand += route[i].demand;
            }

            ride            += data.distances[route[i].id][route[i+1].id];
            justDistance    += data.distances[route[i].id][route[i+1].id];
            batteryUsed     += data.distances[route[i].id][route[i+1].id];
            
            if(batteryUsed > batteryKm){
                violationBattery += (batteryUsed - batteryKm);
            }
        }

        if(ride > route[route.size()-1].twB){                          //janela B
            violationTw += (ride - route[route.size()-1].twB);
            ride = route[route.size()-1].twB;
        }
        
        if(demand > demandCapacity){
            violationDemand += (demand - demandCapacity);
        }

        penality = (violationTw * data.betaTw) + (violationDemand * data.betaDemand) + (violationBattery * data.betaBattery);
        
        objective = justDistance + penality + vehicleCost;
    }
}

void Solution::shake(Data &data, int intensity, int &lastShake){

	int iter = 0, maxIntensity = 4;
	
	if(intensity > maxIntensity){
        intensity   = maxIntensity;
    }
    
    while(iter < intensity){
        uniform_int_distribution<int> distribution(0, 99);
        int select = distribution(generator);

        if(select < 33){
            shakeSwap(data);
            lastShake = 3;
        }else if(select < 66){
            shakeRelocation(data);
            lastShake = 1;
        }else{
            shakeRemoveStation(data);
        }
        iter++;
    }
}

void Solution::shakeSwap(Data &data){
    int v1, v2, r1, r2, k = 0;
    Vehicle car1, car2;

    if((int)vehicles.size() > 1){
    	uniform_int_distribution<int> distribution(0, vehicles.size()-1);
    	do{
    		v1  = distribution(generator);
    		v2  = distribution(generator);
    		if(k > 10){
    			shakeRelocation(data);
    			return;
    		}
    		k++;
    	}while(v1 == v2 || (int)vehicles[v1].route.size() < 3 || (int)vehicles[v2].route.size() < 3);

    	uniform_int_distribution<int> distribution2(1,(int)vehicles[v1].route.size()-2);
	    r1      = distribution2(generator);
	    uniform_int_distribution<int> distribution3(1,(int)vehicles[v2].route.size()-2);
	    r2      = distribution3(generator);
		
		car1    = vehicles[v1];
	    car2    = vehicles[v2];
        
	    swap(car1.route[r1], car2.route[r2]);
        
	    car1.evaluateVehicle(data);
	    car2.evaluateVehicle(data);

	    ride                    = ride - 					vehicles[v1].ride - 					vehicles[v2].ride 					+ car1.ride 					+ car2.ride;
	    demand                  = demand - 					vehicles[v1].demand - 					vehicles[v2].demand 				+ car1.demand 					+ car2.demand;
	    waitTw                  = waitTw - 					vehicles[v1].waitTw - 					vehicles[v2].waitTw 				+ car1.waitTw 					+ car2.waitTw;
	    penality                = penality - 				vehicles[v1].penality - 				vehicles[v2].penality 				+ car1.penality 				+ car2.penality;
	    violationTw             = violationTw - 			vehicles[v1].violationTw - 				vehicles[v2].violationTw 			+ car1.violationTw 				+ car2.violationTw;
	    justDistance            = justDistance - 			vehicles[v1].justDistance - 			vehicles[v2].justDistance 			+ car1.justDistance 			+ car2.justDistance;
	    violationDemand         = violationDemand - 		vehicles[v1].violationDemand - 			vehicles[v2].violationDemand 		+ car1.violationDemand 			+ car2.violationDemand;
	    violationBattery        = violationBattery - 		vehicles[v1].violationBattery - 		vehicles[v2].violationBattery 		+ car1.violationBattery 		+ car2.violationBattery;
	    numberStationsVisited   = numberStationsVisited - 	vehicles[v1].numberStationsVisited - 	vehicles[v2].numberStationsVisited 	+ car1.numberStationsVisited 	+ car2.numberStationsVisited;

	    vehicles[v1]            = car1;
	    vehicles[v2]            = car2;

	    objective = justDistance + vehicleCost + penality;  
    }else{
    	v1 = 0;
    	uniform_int_distribution<int> distribution2(1,(int)vehicles[v1].route.size()-2);
	    r1      = distribution2(generator);
	    uniform_int_distribution<int> distribution3(1,(int)vehicles[v1].route.size()-2);
	    r2      = distribution3(generator);
		
		car1    = vehicles[v1];

	    swap(car1.route[r1], car1.route[r2]);
		    
	    car1.evaluateVehicle(data);

	    ride                    = ride - 					vehicles[v1].ride  					+ car1.ride;
	    demand                  = demand - 					vehicles[v1].demand  				+ car1.demand;
	    waitTw                  = waitTw - 					vehicles[v1].waitTw  				+ car1.waitTw;
	    penality                = penality - 				vehicles[v1].penality  				+ car1.penality;
	    violationTw             = violationTw - 			vehicles[v1].violationTw  			+ car1.violationTw;
	    justDistance            = justDistance - 			vehicles[v1].justDistance  			+ car1.justDistance;
	    violationDemand         = violationDemand - 		vehicles[v1].violationDemand  		+ car1.violationDemand;
	    violationBattery        = violationBattery - 		vehicles[v1].violationBattery  		+ car1.violationBattery;
	    numberStationsVisited   = numberStationsVisited - 	vehicles[v1].numberStationsVisited 	+ car1.numberStationsVisited;

	    vehicles[v1]            = car1;
	    objective = justDistance + vehicleCost + penality;
    }
}

void Solution::shakeRelocation(Data &data){
    int v1, v2, r1, r2;
    Vehicle car1, car2;
    if((int)vehicles.size() > 1){
    	uniform_int_distribution<int> distribution(0, vehicles.size()-1);
    	
    	do{
	        v1  = distribution(generator);
	        v2  = distribution(generator);
	    }while(v1 == v2 || (int)vehicles[v1].route.size() < 3);

	    uniform_int_distribution<int> distribution2(1,(int)vehicles[v1].route.size()-2);
	    r1      = distribution2(generator);

	    if((int)vehicles[v2].route.size() < 3){
	        r2 = 1;
	    }else{
	        uniform_int_distribution<int> distribution3(1,(int)vehicles[v2].route.size()-2);
	        r2      = distribution3(generator);
	    }

	    car1    = vehicles[v1];
	    car2    = vehicles[v2];
        
	    car2.route.insert(car2.route.begin() + r2, car1.route[r1]);
	    car1.route.erase(car1.route.begin() + r1);
	    
	    car1.evaluateVehicle(data);
	    car2.evaluateVehicle(data);

	    ride                    = ride -                vehicles[v1].ride - vehicles[v2].ride + car1.ride + car2.ride;
	    demand                  = demand -              vehicles[v1].demand - vehicles[v2].demand + car1.demand + car2.demand;
	    waitTw                  = waitTw -              vehicles[v1].waitTw - vehicles[v2].waitTw + car1.waitTw + car2.waitTw;
	    penality                = penality -            vehicles[v1].penality - vehicles[v2].penality + car1.penality + car2.penality;
	    violationTw             = violationTw -         vehicles[v1].violationTw - vehicles[v2].violationTw + car1.violationTw + car2.violationTw;
	    justDistance            = justDistance -        vehicles[v1].justDistance - vehicles[v2].justDistance + car1.justDistance + car2.justDistance;
	    violationDemand         = violationDemand -     vehicles[v1].violationDemand - vehicles[v2].violationDemand + car1.violationDemand + car2.violationDemand;
	    violationBattery        = violationBattery -    vehicles[v1].violationBattery - vehicles[v2].violationBattery + car1.violationBattery + car2.violationBattery;
	    numberStationsVisited   = numberStationsVisited - vehicles[v1].numberStationsVisited - vehicles[v2].numberStationsVisited + car1.numberStationsVisited + car2.numberStationsVisited;
	    
	    if((int)vehicles[v1].route.size() > 2 && (int)car1.route.size() < 3){
	        vehicleCost = vehicleCost - car1.vehicleCost;
	        amountVehicles--;
	    }
	    if((int)vehicles[v2].route.size() < 3 && (int)car2.route.size() > 2){
	        vehicleCost = vehicleCost + car2.vehicleCost;
	        amountVehicles++;
	    }
	    
	    vehicles[v1]            = car1;
	    vehicles[v2]            = car2;

	    objective = justDistance + vehicleCost + penality;
    }else{
    	v1  = 0;

	    uniform_int_distribution<int> distribution2(1,(int)vehicles[v1].route.size()-2);
	    do{
	    	r1      = distribution2(generator);
	        r2      = distribution2(generator);
	    }while(r1 >= r2);

	    car1    = vehicles[v1];

	    car1.route.insert(car1.route.begin() + r2+1, car1.route[r1]);
	    car1.route.erase(car1.route.begin() + r1);

	    car1.evaluateVehicle(data);

	    ride                    = ride -                  vehicles[v1].ride 					+ car1.ride;
	    demand                  = demand -                vehicles[v1].demand 					+ car1.demand;
	    waitTw                  = waitTw -                vehicles[v1].waitTw 					+ car1.waitTw;
	    penality                = penality -              vehicles[v1].penality 				+ car1.penality;
	    violationTw             = violationTw -           vehicles[v1].violationTw 				+ car1.violationTw;
	    justDistance            = justDistance -          vehicles[v1].justDistance 			+ car1.justDistance;
	    violationDemand         = violationDemand -       vehicles[v1].violationDemand 			+ car1.violationDemand;
	    violationBattery        = violationBattery -      vehicles[v1].violationBattery 		+ car1.violationBattery;
	    numberStationsVisited   = numberStationsVisited - vehicles[v1].numberStationsVisited 	+ car1.numberStationsVisited;
	    	    
	    vehicles[v1]            = car1;

	    objective = justDistance + vehicleCost + penality;
    }
}

void Solution::shakeRemoveStation(Data &data){
    int v1;
    Vehicle car1;
    Request st1;
    vector< pair < pair < int, int >, int > > stations;
    for(int i = 0; i < (int)vehicles.size(); i++){
        for(int j = 0; j < (int)vehicles[i].route.size(); j++){
            if(vehicles[i].route[j].batteryStation){
                stations.push_back(make_pair(make_pair(i, j), vehicles[i].route[j].id));
            }
        }

    }
    if((int)stations.size() > 0){
        uniform_int_distribution<int> distribution(0, stations.size()-1);
        v1 = distribution(generator);
        st1 = data.requests[stations[v1].second];
        
        for(int i = (int)stations.size() -1; i >= 0; i--){
            if(stations[i].second == st1.id){
                vehicles[stations[i].first.first].route.erase(vehicles[stations[i].first.first].route.begin() + stations[i].first.second);
            }
        }
        evaluateSolution(data);
    }
}

void Solution::interRVND(Data &data, int lastShake){
    double bestObjective = objective;
    int k = 0;
    vector < pair < int, int > > idRemove;
    vector<int> neighbors;
    neighbors.push_back(1);
    neighbors.push_back(2);
    neighbors.push_back(3);
    neighbors.push_back(4);
    neighbors.push_back(5);
    neighbors.push_back(6);
    neighbors.push_back(7);
    
    if(lastShake != 0){
        neighbors.erase(neighbors.begin() + lastShake-1);
    }

    shuffle(neighbors.begin(), neighbors.end(), generator);

    while(true){
        if (k >= (int)neighbors.size()){break;}

        if(neighbors[k] == 1){
            interRelocation(data);

            if(bestObjective - objective > EPS){
                bestObjective = objective;
                k = 0;
                shuffle(neighbors.begin(), neighbors.end(), generator);
            }else{
                k++;
            }
        }else if(neighbors[k] == 2){
            interRelocation2(data);

            if(bestObjective - objective > EPS){
                bestObjective = objective;
                shuffle(neighbors.begin(), neighbors.end(), generator);
                k = 0;
            }else{
                k++;
            }
        }else if(neighbors[k] == 3){
            interSwap(data);
            
            if(bestObjective - objective > EPS){
                bestObjective = objective;
                shuffle(neighbors.begin(), neighbors.end(), generator);
                k = 0;
            }else{
                k++;
            }
        }else if (neighbors[k] == 4){
            interSwap2(data);

            if(bestObjective - objective > EPS){
                bestObjective = objective;
                shuffle(neighbors.begin(), neighbors.end(), generator);
                k = 0;
            }else{
                k++;
            }
        }else if (neighbors[k] == 5){
            addStation(data);
            
            if(bestObjective - objective > EPS){
                bestObjective = objective;
                shuffle(neighbors.begin(), neighbors.end(), generator);
                k = 0;
            }else{
                k++;
            }
        }else if (neighbors[k] == 6){
            removeStation(data);

            if(bestObjective - objective > EPS){
                bestObjective = objective;
                shuffle(neighbors.begin(), neighbors.end(), generator);
                k = 0;
            }else{
                k++;
            }
        }else if (neighbors[k] == 7){
            interSwap2x1(data);

            if(bestObjective - objective > EPS){
                bestObjective = objective;
                shuffle(neighbors.begin(), neighbors.end(), generator);
                k = 0;
            }else{
                k++;
            }
        }else{
            k++;
        }
    }
}

void Solution::addStation(Data &data){
    Solution solution;
    Vehicle car, bestCar;
    bool improve = false;
    double bestObjective = objective, newObjective, newJustDistance, newPenality;
    int idCar;

    for(int i = 1; i <= data.numBatteryStations; i++){
        for(int k = 0; k < (int)vehicles.size(); k++){
            if((int)vehicles[k].route.size() > 2){
                car = vehicles[k];
                for(int j = 1; j < (int)vehicles[k].route.size(); j++){
                    if(!(j == 1 && strcmp(data.requests[i].sigla, "S0") == 0)){
                        if(!(j == (int)vehicles[k].route.size() -1 && strcmp(data.requests[i].sigla, "S0") == 0)){

                            car.route.insert(car.route.begin() + j, data.requests[i]);
                            car.evaluateVehicle(data);

                            newJustDistance = justDistance + car.justDistance - vehicles[k].justDistance;
                            newPenality     = penality     + car.penality     - vehicles[k].penality;

                            newObjective = newJustDistance + vehicleCost + newPenality;

                            if(bestObjective - newObjective > EPS){
                                improve = true;
                                bestCar = car;
                                idCar = k;
                                bestObjective = newObjective;
                            }
                            car = vehicles[k];
                        }
                    }
                }
            }
        }
    }

    if(improve){
        ride                    = ride -                    vehicles[idCar].ride                   + bestCar.ride;
        waitTw                  = waitTw -                  vehicles[idCar].waitTw                 + bestCar.waitTw;
        penality                = penality -                vehicles[idCar].penality               + bestCar.penality;
        violationTw             = violationTw -             vehicles[idCar].violationTw            + bestCar.violationTw;
        justDistance            = justDistance -            vehicles[idCar].justDistance           + bestCar.justDistance;
        violationDemand         = violationDemand -         vehicles[idCar].violationDemand        + bestCar.violationDemand;
        violationBattery        = violationBattery -        vehicles[idCar].violationBattery       + bestCar.violationBattery;
        numberStationsVisited   = numberStationsVisited -   vehicles[idCar].numberStationsVisited  + bestCar.numberStationsVisited;
        
        swap(vehicles[idCar], bestCar);
        
        objective = justDistance + penality + vehicleCost;
    }
}

void Solution::removeStation(Data &data){
    Vehicle car, bestCar;
    int idCar;
    bool improve = false;
    double newObjective, bestObjective = objective, newPenality, newJustDistance, newVehiclesCost;
    vector < pair < int, int > > idRemove;

    for(int i = 0; i < (int)vehicles.size(); i++){
        if((int)vehicles[i].route.size() > 2){
            for(int j = 1; j < (int)vehicles[i].route.size() - 1; j++){
                if(vehicles[i].route[j].batteryStation){
                    if(vehicles[i].route[j-1].id == vehicles[i].route[j].id){
                        idRemove.push_back(make_pair(i, j-1));
                    }
                }
            }
        }
    }

    for(int i = (int)idRemove.size() -1; i >= 0; i--){
        vehicles[idRemove[i].first].route.erase(vehicles[idRemove[i].first].route.begin() + idRemove[i].second);
        vehicles[idRemove[i].first].evaluateVehicle(data);
    }

    for(int i = 0; i < (int)vehicles.size(); i++){
        if((int)vehicles[i].route.size() > 2){
            for(int j = 1; j < (int)vehicles[i].route.size() - 1; j++){
                newVehiclesCost = vehicleCost;
                if(vehicles[i].route[j].batteryStation){
                    car = vehicles[i];
                    
                    car.route.erase(car.route.begin() + j);
                    car.evaluateVehicle(data);

                    newJustDistance = justDistance + car.justDistance - vehicles[i].justDistance;
                    newPenality     = penality     + car.penality     - vehicles[i].penality;

                    newObjective = newJustDistance + newVehiclesCost + newPenality;

                    if(bestObjective - newObjective > EPS){
                        improve = true;
                        bestCar = car;
                        idCar = i;
                        bestObjective = newObjective;
                    }
                }
            }
        }
    }

    if(improve){
        ride                    = ride -                    vehicles[idCar].ride                   + bestCar.ride;
        waitTw                  = waitTw -                  vehicles[idCar].waitTw                 + bestCar.waitTw;
        penality                = penality -                vehicles[idCar].penality               + bestCar.penality;
        violationTw             = violationTw -             vehicles[idCar].violationTw            + bestCar.violationTw;
        justDistance            = justDistance -            vehicles[idCar].justDistance           + bestCar.justDistance;
        violationDemand         = violationDemand -         vehicles[idCar].violationDemand        + bestCar.violationDemand;
        violationBattery        = violationBattery -        vehicles[idCar].violationBattery       + bestCar.violationBattery;
        numberStationsVisited   = numberStationsVisited -   vehicles[idCar].numberStationsVisited  + bestCar.numberStationsVisited;
        
        swap(vehicles[idCar], bestCar);
        
        objective = justDistance + penality + vehicleCost;
    }
}

void Solution::interRelocation(Data &data){
    int bestV1, bestV2, contEmptyVehicle = 0;
    double newTravelledDistance, newPenality, bestObjective = objective, newObjective, newVehiclesCost = vehicleCost;
    bool improve = true;
    Vehicle car1, car2, bestCar1, bestCar2;

    vector < pair < pair < int, int >, pair < int, int > > > vt;
    improve = false;

    for(int i = 0; i < (int)vehicles.size();i++) {
        if((int)vehicles[i].route.size() > 2){
            for(int j = 0; j < (int)vehicles.size(); j++) {
                if(i != j){
                    if((int)vehicles[j].route.size() > 3 || contEmptyVehicle == 0){
                        for(int r1 = 1; r1 < (int)vehicles[i].route.size()-1; r1++) {
                            if((int)vehicles[j].route.size() < 3){
                                vt.push_back(make_pair(make_pair(i, r1), make_pair(j, 1)));
                            }else{
                                for(int r2 = 1; r2 < (int)vehicles[j].route.size()-1; r2++) {
                                    vt.push_back(make_pair(make_pair(i, r1), make_pair(j, r2)));
                                }                        
                            }
                        }
                        contEmptyVehicle++;
                    }
                }
            }
        }
    }
    
    for(int i = 0; i < (int)vt.size(); i++){
        newVehiclesCost = vehicleCost;
            
        car1                    = vehicles[vt[i].first.first];
        car2                    = vehicles[vt[i].second.first];

        car2.route.insert(car2.route.begin() + vt[i].second.second, car1.route[vt[i].first.second]);
        car1.route.erase(car1.route.begin() + vt[i].first.second);
        
        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);
    
        newTravelledDistance    = justDistance      + car1.justDistance      + car2.justDistance -      vehicles[vt[i].first.first].justDistance -      vehicles[vt[i].second.first].justDistance;
        newPenality             = penality          + car1.penality          + car2.penality -          vehicles[vt[i].first.first].penality -          vehicles[vt[i].second.first].penality;
        
        if(vehicles[vt[i].first.first].route.size() > 2 && car1.route.size() < 3){
            newVehiclesCost = vehicleCost - car1.vehicleCost;
        }

        if (vehicles[vt[i].second.first].route.size() < 3 && car2.route.size() > 2){
            newVehiclesCost = vehicleCost + car2.vehicleCost;
        }

        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first;
            bestV2      = vt[i].second.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }
    }

    if(improve){
        if(bestCar1.route.size() > 2){
            bestCar1.intraRVND(data);
        }

        bestCar2.intraRVND(data);

        ride                    = ride -                    vehicles[bestV1].ride -                  vehicles[bestV2].ride                  + bestCar1.ride                  + bestCar2.ride;
        demand                  = demand -                  vehicles[bestV1].demand -                vehicles[bestV2].demand                + bestCar1.demand                + bestCar2.demand;
        waitTw                  = waitTw -                  vehicles[bestV1].waitTw -                vehicles[bestV2].waitTw                + bestCar1.waitTw                + bestCar2.waitTw;
        penality                = penality -                vehicles[bestV1].penality -              vehicles[bestV2].penality              + bestCar1.penality              + bestCar2.penality;
        violationTw             = violationTw -             vehicles[bestV1].violationTw -           vehicles[bestV2].violationTw           + bestCar1.violationTw           + bestCar2.violationTw;
        justDistance            = justDistance -            vehicles[bestV1].justDistance -          vehicles[bestV2].justDistance          + bestCar1.justDistance          + bestCar2.justDistance;
        violationDemand         = violationDemand -         vehicles[bestV1].violationDemand -       vehicles[bestV2].violationDemand       + bestCar1.violationDemand       + bestCar2.violationDemand;
        violationBattery        = violationBattery -        vehicles[bestV1].violationBattery -      vehicles[bestV2].violationBattery      + bestCar1.violationBattery      + bestCar2.violationBattery;
        numberStationsVisited   = numberStationsVisited -   vehicles[bestV1].numberStationsVisited - vehicles[bestV2].numberStationsVisited + bestCar1.numberStationsVisited + bestCar2.numberStationsVisited;

        if(vehicles[bestV1].route.size() > 2 && bestCar1.route.size() < 3){
            vehicleCost = vehicleCost - bestCar1.vehicleCost;
            amountVehicles--;
        }

        if(vehicles[bestV2].route.size() < 3 && bestCar2.route.size() > 2){
            vehicleCost = vehicleCost + bestCar2.vehicleCost;
            amountVehicles++;
        }
        
        objective               = justDistance + vehicleCost + penality;

        swap(vehicles[bestV1], bestCar1);
        swap(vehicles[bestV2], bestCar2);
    }
}

void Solution::interRelocation2(Data &data){
    int bestV1 = -1, bestV2 = -1, contEmptyVehicle = 0, v1, v2, r1, r1_2, r2;
    double newTravelledDistance, newPenality, bestObjective = objective, newObjective, newVehiclesCost = vehicleCost;
    bool improve = true;
    Vehicle car1, car2, bestCar1, bestCar2;

    vector < pair < pair < pair < int, int >, int >, pair < int, int > > > vt;
    improve = false;
    for(int i = 0; i < (int)vehicles.size();i++) {
        if((int)vehicles[i].route.size() > 3){
            for(int j = 0; j < (int)vehicles.size(); j++) {
                if(i != j){
                    if((int)vehicles[j].route.size() > 2 || contEmptyVehicle == 0){
                        for(int r1 = 1; r1 < (int)vehicles[i].route.size()-2; r1++) {
                            if((int)vehicles[j].route.size() < 3){
                                vt.push_back(make_pair(make_pair(make_pair(i, r1), r1+1), make_pair(j, 1)));
                            }else{
                                for(int r2 = 1; r2 < (int)vehicles[j].route.size()-1; r2++) {
                                    vt.push_back(make_pair(make_pair(make_pair(i, r1), r1+1), make_pair(j, r2)));
                                }                        
                            }
                        }
                        contEmptyVehicle++;
                    }
                }
            }
        }
    }

    for(int i = 0; i < (int)vt.size(); i++){
        newVehiclesCost = vehicleCost;
        v1 = vt[i].first.first.first;
        v2 = vt[i].second.first;
        r1 = vt[i].first.first.second;
        r1_2 = vt[i].first.second;
        r2 = vt[i].second.second;

        car1                    = vehicles[vt[i].first.first.first];
        car2                    = vehicles[vt[i].second.first];
        
        car2.route.insert(car2.route.begin() + r2, car1.route[r1_2]);
        car2.route.insert(car2.route.begin() + r2, car1.route[r1]);
        car1.route.erase(car1.route.begin() + r1_2);
        car1.route.erase(car1.route.begin() + r1);
        
        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);
    
        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first.first].justDistance - vehicles[vt[i].second.first].justDistance;
        newPenality             = penality          + car1.penality          + car2.penality -          vehicles[vt[i].first.first.first].penality -          vehicles[vt[i].second.first].penality;
        
        if(vehicles[v1].route.size() > 2 && car1.route.size() < 3){
            newVehiclesCost = vehicleCost - car1.vehicleCost;
        }

        if (vehicles[v2].route.size() < 3 && car2.route.size() > 2){
            newVehiclesCost = vehicleCost + car2.vehicleCost;
        }

        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first.first;
            bestV2      = vt[i].second.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }

        // Inverte
        swap(car2.route[r2], car2.route[r2+1]);

        car2.evaluateVehicle(data);
    
        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first.first].justDistance - vehicles[vt[i].second.first].justDistance;
        newPenality             = penality          + car1.penality          + car2.penality -          vehicles[vt[i].first.first.first].penality -          vehicles[vt[i].second.first].penality;
        
        if(vehicles[vt[i].first.first.first].route.size() > 2 && car1.route.size() < 3){
            newVehiclesCost = vehicleCost - car1.vehicleCost;
        }

        if (vehicles[vt[i].second.first].route.size() < 3 && car2.route.size() > 2){
            newVehiclesCost = vehicleCost + car2.vehicleCost;
        }

        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first.first;
            bestV2      = vt[i].second.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }
    }

    if(improve){
        if(bestCar1.route.size() > 2){
            bestCar1.intraRVND(data);
        }

        bestCar2.intraRVND(data);

        ride                    = ride -                    vehicles[bestV1].ride -                  vehicles[bestV2].ride                  + bestCar1.ride                  + bestCar2.ride;
        demand                  = demand -                  vehicles[bestV1].demand -                vehicles[bestV2].demand                + bestCar1.demand                + bestCar2.demand;
        waitTw                  = waitTw -                  vehicles[bestV1].waitTw -                vehicles[bestV2].waitTw                + bestCar1.waitTw                + bestCar2.waitTw;
        penality                = penality -                vehicles[bestV1].penality -              vehicles[bestV2].penality              + bestCar1.penality              + bestCar2.penality;
        violationTw             = violationTw -             vehicles[bestV1].violationTw -           vehicles[bestV2].violationTw           + bestCar1.violationTw           + bestCar2.violationTw;
        justDistance            = justDistance -            vehicles[bestV1].justDistance -          vehicles[bestV2].justDistance          + bestCar1.justDistance          + bestCar2.justDistance;
        violationDemand         = violationDemand -         vehicles[bestV1].violationDemand -       vehicles[bestV2].violationDemand       + bestCar1.violationDemand       + bestCar2.violationDemand;
        violationBattery        = violationBattery -        vehicles[bestV1].violationBattery -      vehicles[bestV2].violationBattery      + bestCar1.violationBattery      + bestCar2.violationBattery;
        numberStationsVisited   = numberStationsVisited -   vehicles[bestV1].numberStationsVisited - vehicles[bestV2].numberStationsVisited + bestCar1.numberStationsVisited + bestCar2.numberStationsVisited;
        
        if(vehicles[bestV1].route.size() > 2 && bestCar1.route.size() < 3){
            vehicleCost = vehicleCost - bestCar1.vehicleCost;
            amountVehicles--;
        }
        
        if(vehicles[bestV2].route.size() < 3 && bestCar2.route.size() > 2){
            vehicleCost = vehicleCost + bestCar2.vehicleCost;
            amountVehicles++;
        }

        objective               = justDistance + vehicleCost + penality;
        
        swap(vehicles[bestV1], bestCar1);
        swap(vehicles[bestV2], bestCar2);
    }
}

void Solution::interSwap(Data &data){
    int bestV1, bestV2;
    double newTravelledDistance, newPenality, bestObjective = objective, newObjective, newVehiclesCost = vehicleCost;
    bool improve = false;
    Vehicle car1, car2, bestCar1, bestCar2;

    vector < pair < pair < int, int >, pair < int, int > > > vt;
    for(int i = 0; i < (int)vehicles.size();i++) {
        if((int)vehicles[i].route.size() > 2){
            for(int j = 0; j < (int)vehicles.size(); j++) {
                if(i != j){
                    for(int r1 = 1; r1 < (int)vehicles[i].route.size()-1; r1++) {
                        if((int)vehicles[j].route.size() > 2){
                            for(int r2 = 1; r2 < (int)vehicles[j].route.size()-1; r2++) {
                                vt.push_back(make_pair(make_pair(i, r1), make_pair(j, r2)));
                            }                        
                        }
                    }
                }
            }
        }
    }

    for(int i = 0; i < (int)vt.size(); i++){
        car1                    = vehicles[vt[i].first.first];
        car2                    = vehicles[vt[i].second.first];

        swap(car1.route[vt[i].first.second], car2.route[vt[i].second.second]);

        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);
    
        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first].justDistance - vehicles[vt[i].second.first].justDistance;
        newPenality             = penality          + car1.penality          + car2.penality -          vehicles[vt[i].first.first].penality -          vehicles[vt[i].second.first].penality;
        
        newObjective = newTravelledDistance + newVehiclesCost + newPenality;
        
        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first;
            bestV2      = vt[i].second.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }
    }

    if(improve){

        bestCar1.intraRVND(data);
        bestCar2.intraRVND(data);

        ride                    = ride -                    vehicles[bestV1].ride -                  vehicles[bestV2].ride                  + bestCar1.ride                  + bestCar2.ride;
        demand                  = demand -                  vehicles[bestV1].demand -                vehicles[bestV2].demand                + bestCar1.demand                + bestCar2.demand;
        waitTw                  = waitTw -                  vehicles[bestV1].waitTw -                vehicles[bestV2].waitTw                + bestCar1.waitTw                + bestCar2.waitTw;
        penality                = penality -                vehicles[bestV1].penality -              vehicles[bestV2].penality              + bestCar1.penality              + bestCar2.penality;
        violationTw             = violationTw -             vehicles[bestV1].violationTw -           vehicles[bestV2].violationTw           + bestCar1.violationTw           + bestCar2.violationTw;
        justDistance            = justDistance -            vehicles[bestV1].justDistance -          vehicles[bestV2].justDistance          + bestCar1.justDistance          + bestCar2.justDistance;
        violationDemand         = violationDemand -         vehicles[bestV1].violationDemand -       vehicles[bestV2].violationDemand       + bestCar1.violationDemand       + bestCar2.violationDemand;
        violationBattery        = violationBattery -        vehicles[bestV1].violationBattery -      vehicles[bestV2].violationBattery      + bestCar1.violationBattery      + bestCar2.violationBattery;
        numberStationsVisited   = numberStationsVisited -   vehicles[bestV1].numberStationsVisited - vehicles[bestV2].numberStationsVisited + bestCar1.numberStationsVisited + bestCar2.numberStationsVisited;

        objective               = justDistance + vehicleCost + penality;

        swap(vehicles[bestV1], bestCar1);
        swap(vehicles[bestV2], bestCar2);
    }
}

void Solution::interSwap2(Data &data){
    int bestV1, bestV2;
    double newTravelledDistance, newPenality, bestObjective = objective, newObjective, newVehiclesCost = vehicleCost;
    bool improve = false;
    Vehicle car1, car2, bestCar1, bestCar2;

    vector < pair < pair < pair < int, int >, int >, pair < pair < int, int >, int > > > vt;
    for(int i = 0; i < (int)vehicles.size(); i++) {
        for(int j = 0; j < (int)vehicles.size(); j++) {
            if(i != j){
                if((int)vehicles[i].route.size() > 3 && (int)vehicles[j].route.size() > 3){                
                    for(int r1 = 1; r1 < (int)vehicles[i].route.size()-2; r1++) {                    
                        for(int r2 = 1; r2 < (int)vehicles[j].route.size()-2; r2++) {
                            vt.push_back( make_pair( make_pair( make_pair(i, r1), r1+1), make_pair( make_pair(j, r2), r2+1)));
                        }
                    }
                }
            }
        }
    }


    for(int i = 0; i < (int)vt.size(); i++){

        car1                    = vehicles[vt[i].first.first.first];
        car2                    = vehicles[vt[i].second.first.first];

        swap(car1.route[vt[i].first.first.second], car2.route[vt[i].second.first.second]);
        swap(car1.route[vt[i].first.second], car2.route[vt[i].second.second]);
        
        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);

        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first.first].justDistance - vehicles[vt[i].second.first.first].justDistance;
        newPenality             = penality     + car1.penality     + car2.penality -     vehicles[vt[i].first.first.first].penality -     vehicles[vt[i].second.first.first].penality;
        
        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first.first;
            bestV2      = vt[i].second.first.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }
        
        // Inverte car 1
        swap(car1.route[vt[i].first.second], car1.route[vt[i].first.first.second]);
        
        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);

        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first.first].justDistance - vehicles[vt[i].second.first.first].justDistance;
        newPenality             = penality     + car1.penality     + car2.penality -     vehicles[vt[i].first.first.first].penality -     vehicles[vt[i].second.first.first].penality;
        
        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first.first;
            bestV2      = vt[i].second.first.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }

        // Inverte Car 2
        swap(car1.route[vt[i].first.second], car1.route[vt[i].first.first.second]);
        swap(car2.route[vt[i].second.first.second], car2.route[vt[i].second.second]);

        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);

        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first.first].justDistance - vehicles[vt[i].second.first.first].justDistance;
        newPenality             = penality     + car1.penality     + car2.penality -     vehicles[vt[i].first.first.first].penality -     vehicles[vt[i].second.first.first].penality;
        
        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first.first;
            bestV2      = vt[i].second.first.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }
    }

    if(improve){
        bestCar1.intraRVND(data);
        bestCar2.intraRVND(data);
        
        ride                    = ride -                    vehicles[bestV1].ride -                  vehicles[bestV2].ride                  + bestCar1.ride                  + bestCar2.ride;
        demand                  = demand -                  vehicles[bestV1].demand -                vehicles[bestV2].demand                + bestCar1.demand                + bestCar2.demand;
        waitTw                  = waitTw -                  vehicles[bestV1].waitTw -                vehicles[bestV2].waitTw                + bestCar1.waitTw                + bestCar2.waitTw;
        penality                = penality -                vehicles[bestV1].penality -              vehicles[bestV2].penality              + bestCar1.penality              + bestCar2.penality;
        violationTw             = violationTw -             vehicles[bestV1].violationTw -           vehicles[bestV2].violationTw           + bestCar1.violationTw           + bestCar2.violationTw;
        justDistance            = justDistance -            vehicles[bestV1].justDistance -          vehicles[bestV2].justDistance          + bestCar1.justDistance          + bestCar2.justDistance;
        violationDemand         = violationDemand -         vehicles[bestV1].violationDemand -       vehicles[bestV2].violationDemand       + bestCar1.violationDemand       + bestCar2.violationDemand;
        violationBattery        = violationBattery -        vehicles[bestV1].violationBattery -      vehicles[bestV2].violationBattery      + bestCar1.violationBattery      + bestCar2.violationBattery;
        numberStationsVisited   = numberStationsVisited -   vehicles[bestV1].numberStationsVisited - vehicles[bestV2].numberStationsVisited + bestCar1.numberStationsVisited + bestCar2.numberStationsVisited;

        objective               = justDistance + vehicleCost + penality;

        swap(vehicles[bestV1], bestCar1);
        swap(vehicles[bestV2], bestCar2);
    }
}

void Solution::interSwap2x1(Data &data){
    int bestV1, bestV2;
    double newTravelledDistance, newPenality, bestObjective = objective, newObjective, newVehiclesCost = vehicleCost;
    bool improve = false;
    Vehicle car1, car2, bestCar1, bestCar2;

    vector < pair < pair < pair < int, int >, int >, pair < pair < int, int >, int > > > vt;
    for(int i = 0; i < (int)vehicles.size(); i++) {
        for(int j = 0; j < (int)vehicles.size(); j++) {
            if(i != j){
                if((int)vehicles[i].route.size() > 3 && (int)vehicles[j].route.size() > 2){                
                    for(int r1 = 1; r1 < (int)vehicles[i].route.size()-2; r1++) {                    
                        for(int r2 = 1; r2 < (int)vehicles[j].route.size()-1; r2++) {
                            vt.push_back( make_pair( make_pair( make_pair(i, r1), r1+1), make_pair( make_pair(j, r2), r2+1)));
                        }
                    }
                }
            }
        }
    }


    for(int i = 0; i < (int)vt.size(); i++){

        car1                    = vehicles[vt[i].first.first.first];
        car2                    = vehicles[vt[i].second.first.first];

        swap(car1.route[vt[i].first.first.second], car2.route[vt[i].second.first.second]);
        car2.route.insert(car2.route.begin() + vt[i].second.second, car1.route[vt[i].first.second]);
        car1.route.erase(car1.route.begin() + vt[i].first.second);
        
        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);

        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first.first].justDistance - vehicles[vt[i].second.first.first].justDistance;
        newPenality             = penality     + car1.penality     + car2.penality -     vehicles[vt[i].first.first.first].penality -     vehicles[vt[i].second.first.first].penality;
        
        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first.first;
            bestV2      = vt[i].second.first.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }
        
        // Inverte car 1
        swap(car2.route[vt[i].second.first.second], car2.route[vt[i].second.second]);

        car1.evaluateVehicle(data);
        car2.evaluateVehicle(data);

        newTravelledDistance    = justDistance + car1.justDistance + car2.justDistance - vehicles[vt[i].first.first.first].justDistance - vehicles[vt[i].second.first.first].justDistance;
        newPenality             = penality     + car1.penality     + car2.penality -     vehicles[vt[i].first.first.first].penality -     vehicles[vt[i].second.first.first].penality;
        
        newObjective = newTravelledDistance + newVehiclesCost + newPenality;

        if(bestObjective - newObjective > EPS){
            bestV1      = vt[i].first.first.first;
            bestV2      = vt[i].second.first.first;
            bestCar1    = car1;
            bestCar2    = car2;
            improve     = true;
            bestObjective = newObjective;
        }
    }

    if(improve){
        bestCar1.intraRVND(data);
        bestCar2.intraRVND(data);
        
        ride                    = ride -                    vehicles[bestV1].ride -                  vehicles[bestV2].ride                  + bestCar1.ride                  + bestCar2.ride;
        demand                  = demand -                  vehicles[bestV1].demand -                vehicles[bestV2].demand                + bestCar1.demand                + bestCar2.demand;
        waitTw                  = waitTw -                  vehicles[bestV1].waitTw -                vehicles[bestV2].waitTw                + bestCar1.waitTw                + bestCar2.waitTw;
        penality                = penality -                vehicles[bestV1].penality -              vehicles[bestV2].penality              + bestCar1.penality              + bestCar2.penality;
        violationTw             = violationTw -             vehicles[bestV1].violationTw -           vehicles[bestV2].violationTw           + bestCar1.violationTw           + bestCar2.violationTw;
        justDistance            = justDistance -            vehicles[bestV1].justDistance -          vehicles[bestV2].justDistance          + bestCar1.justDistance          + bestCar2.justDistance;
        violationDemand         = violationDemand -         vehicles[bestV1].violationDemand -       vehicles[bestV2].violationDemand       + bestCar1.violationDemand       + bestCar2.violationDemand;
        violationBattery        = violationBattery -        vehicles[bestV1].violationBattery -      vehicles[bestV2].violationBattery      + bestCar1.violationBattery      + bestCar2.violationBattery;
        numberStationsVisited   = numberStationsVisited -   vehicles[bestV1].numberStationsVisited - vehicles[bestV2].numberStationsVisited + bestCar1.numberStationsVisited + bestCar2.numberStationsVisited;

        objective               = justDistance + vehicleCost + penality;

        swap(vehicles[bestV1], bestCar1);
        swap(vehicles[bestV2], bestCar2);
    }
}

void Vehicle::intraRVND(Data &data){
    double bestObjective = justDistance + penality + vehicleCost, newObjective;
    int count = 2, i;
    vector<int> vR;

    for(i = 0; i <= count; i++){
        vR.push_back(i);
    }
    
    shuffle(vR.begin(), vR.end(), generator);
    count = 0;
    
    while(1){
        if (count >= (int)vR.size()){break;}
        
        if(vR[count] == 0){
            intraRealocation(data);

            newObjective = justDistance + penality + vehicleCost;

            if(bestObjective - newObjective > EPS){
                bestObjective = newObjective;
                count = 0;
                shuffle(vR.begin(), vR.end(), generator);
            }else{
                count++;
            }
         }else if (vR[count] == 1){
            intraShift2(data);
            
            newObjective = justDistance + penality + vehicleCost;

            if(bestObjective - newObjective > EPS){
                bestObjective = newObjective;
                count = 0;
                shuffle(vR.begin(), vR.end(), generator);
            }else{
                count++;
            }
        }else if (vR[count] == 2){
            intraSwap(data);
            
            newObjective = justDistance + penality + vehicleCost;

            if(bestObjective - newObjective > EPS){
                bestObjective = newObjective;
                count = 0;
                shuffle(vR.begin(), vR.end(), generator);
            }else{
                count++;
            }
        }
    }
}

void Vehicle::intraSwap(Data &data) {
    Vehicle inicialVehicle = *this;
    int i, j, bestI = -1, bestJ = -1;
    double bestObjective = justDistance + penality + vehicleCost, newObjective;
    vector < pair < int, int > > vt; 
    
    inicialVehicle = *this;
    bestI = -1, bestJ = -1;

    for (i = 1; i < (int)route.size()-1; i++) {
        for (j = i+1; j < (int)route.size()-1; j++) {
            vt.push_back(make_pair(i, j));
        }
    }

    for (i = 0; i < (int)vt.size(); i++) {
        if(vt[i].first != vt[i].second){
            
            swap(route[vt[i].first], route[vt[i].second]);
            
            evaluateVehicle(data);
            
            newObjective       = justDistance + penality + vehicleCost;
            
            if(bestObjective - newObjective > EPS){
                bestObjective = newObjective;
                bestI = vt[i].first;
                bestJ = vt[i].second;
            }

            *this = inicialVehicle;
        }
    }

    if(bestI != -1){
        swap(route[bestI], route[bestJ]);
        evaluateVehicle(data);
    }

}

void Vehicle::intraShift2(Data &data) {
    Vehicle inicialVehicle = *this;
    int i, j, bestI = -1, bestJ = -1, i2;
    double bestObjective = justDistance + penality + vehicleCost, newObjective;
    vector < pair < int, int > > vt; 
    
    inicialVehicle = *this;
    bestI = -1, bestJ = -1;

    for (i = 1; i < (int)route.size()-1; i++) {
        for (j = i+2; j < (int)route.size()-1; j++) {
            vt.push_back(make_pair(i, j));
        }
    }
    
    for (i = 0; i < (int)vt.size(); i++) {
        i2 = vt[i].first+1;
           
        route.insert(route.begin()+vt[i].second+1, route[vt[i].first]);
        route.insert(route.begin()+vt[i].second+2, route[i2]);
        route.erase(route.begin()+i2);
        route.erase(route.begin()+vt[i].first);
        
        evaluateVehicle(data);
        
        newObjective       = justDistance + penality + vehicleCost;
        
        if(bestObjective - newObjective > EPS){
            bestObjective = newObjective;
            bestI = vt[i].first;
            bestJ = vt[i].second;
        }

        *this = inicialVehicle;
    }

    if(bestI != -1){
        route.insert(route.begin()+bestJ+1, route[bestI]);
        route.insert(route.begin()+bestJ+2, route[bestI+1]);
        route.erase(route.begin()+bestI+1);
        route.erase(route.begin()+bestI);
        evaluateVehicle(data);
    }
    
}

void Vehicle::intraRealocation(Data &data) {
    Vehicle inicialVehicle = *this;
    int i, j, bestI = -1, bestJ = -1;
    double bestObjective = justDistance + penality + vehicleCost, newObjective;
    vector < pair < int, int > > vt; 

    bestI = -1, bestJ = -1;
    for (i = 1; i < (int)route.size()-1; i++) {
        for (j = i+1; j < (int)route.size()-1; j++) {
            vt.push_back(make_pair(i, j));
        }
    }

    for (i = 0; i < (int)vt.size(); i++) {
        route.insert(route.begin()+vt[i].second+1, route[vt[i].first]);
        route.erase(route.begin()+vt[i].first);

        evaluateVehicle(data);

        newObjective       = justDistance + penality + vehicleCost;

        if(bestObjective - newObjective > EPS){
            bestObjective = newObjective;
            bestI = vt[i].first;
            bestJ = vt[i].second;
        }

        *this = inicialVehicle;
    }

    if(bestI != -1){
        route.insert(route.begin()+bestJ+1, route[bestI]);
        route.erase(route.begin()+bestI);
        evaluateVehicle(data);
    }
}

void Solution::showSolution() {
    int numVehicles = 0;
    for (int i = 0; i < (int)vehicles.size(); i++) {
        if((int)vehicles[i].route.size() < 3){
            continue;
        }
        numVehicles++;
        for (int j = 0; j < (int)vehicles[i].route.size(); j++) {
            if(vehicles[i].route[j].batteryStation){
                cout << vehicles[i].route[j].sigla << "(S) - ";
            }else{
                cout << vehicles[i].route[j].sigla << " - ";
            }
        }
        cout << endl;
        cout << "Demand: " << vehicles[i].demand << " | "; 
        cout << "   Capacity: " << vehicles[i].demandCapacity << " | ";
        cout << "   VTW: " << vehicles[i].violationTw << " | " << "     VD: " << vehicles[i].violationDemand << " | " << "     VB: " << vehicles[i].violationBattery << " | ";
        cout << "   Ride: " << vehicles[i].ride << endl;
    }
    cout << "Just Distance: " << justDistance << endl;
    cout << "Objective Solution: " << objective << endl;
    cout << "Amount Vehicles: " << numVehicles << endl;
    amountVehicles = numVehicles;
}

void Vehicle::showRoute() {
    for (int j = 0; j < (int)route.size(); j++) {
        if(route[j].batteryStation){
            cout << route[j].sigla << "(S) - ";
        }else{
            cout << route[j].id << " - ";
        }
    }
    cout << endl;

    cout << "Demand: " << demand << " | "; 
    cout << "Capacity: " << demandCapacity << " | ";
    cout << "   VTW: " << violationTw << " | " << "     VD: " << violationDemand << " | " << "     VB: " << violationBattery << " | ";
    cout << "Just Distance: " << justDistance << endl;
    cout << "ViolationBattery: " << violationBattery << endl;
}
