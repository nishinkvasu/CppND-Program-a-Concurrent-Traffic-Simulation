#include <iostream>
#include <random>
#include <ctime>

#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);
    _condVar.wait(uLock, [this](){return !_queue.empty();});

    T msg = std::move(_queue.back());
    // _queue.pop_back();
    _queue.clear(); 
    // recommendation to make outer interrsections work - but queue
    // https://knowledge.udacity.com/questions/267031
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.emplace_back(std::move(msg));
    // std::cout << " Message added to the queue \n";
    _condVar.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight() : _currentPhase(TrafficLightPhase::kRed)
{
    //_currentPhase = TrafficLightPhase::kRed;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true){
        TrafficLightPhase phase = _msgQueue.receive();
        if(phase == TrafficLightPhase::kGreen){
            // std::cout << "Test green \n";
            break;
        }
        else{
                // std::cout << "Test red \n";
        }
    }

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); // this - object on which it needs to be called
    
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    long cycleDuration;
    // init stop watch
    lastUpdate = std::chrono::system_clock::now();

    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4000.0, 6000.0);
    cycleDuration = distr(eng);

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        
        // std::cout << "Cycle Duration " << cycleDuration << " thread id " << std::this_thread::get_id() << "\n";

        if (timeSinceLastUpdate >= cycleDuration){
            // Toggle current phase
            if(_currentPhase == TrafficLightPhase::kRed){
                _currentPhase = TrafficLightPhase::kGreen;
                // std::cout << "Test green \n";
            }
            else{
                _currentPhase = TrafficLightPhase::kRed;
                // std::cout << "Test red \n";
            }

            // std::cout << "Test \n";
            // TrafficLightPhase phase = _currentPhase;
            // update message queue through move semantics; //TODO - done
            _msgQueue.send(std::move(_currentPhase));

            // reset the stop-watch
            lastUpdate = std::chrono::system_clock::now();

            cycleDuration = distr(eng);
        }
        
    }

}

