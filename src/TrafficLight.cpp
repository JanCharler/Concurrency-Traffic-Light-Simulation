#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex>  lck(_mtx);

    _cnd.wait(lck, [this](){return this->_queue.empty() == false;});

    T val = std::move(_queue.front()); _queue.pop_front();
    return val;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtx);
    _queue.emplace_back(std::move(msg));
    _cnd.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        TrafficLightPhase phase = _messages.receive();
        if (phase == TrafficLightPhase::green){
            break;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be 
    // started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread th(&TrafficLight::cycleThroughPhases, this);
    threads.emplace_back(std::move(th));
}

int getRandomDuration(int min, int max){
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}
// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    auto t0 = std::chrono::steady_clock::now();
    auto t1 = std::chrono::steady_clock::now();
    int duration = getRandomDuration(4000, 6000); 
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        auto t0 = std::chrono::steady_clock::now();
        if ( std::chrono::duration_cast<std::chrono::milliseconds>(t0-t1).count() > std::chrono::milliseconds{duration}.count()){
            
            // toggle the traffic lights
            if (_currentPhase == TrafficLightPhase::green) _currentPhase = TrafficLightPhase::red;
            else _currentPhase = TrafficLightPhase::green;
            

            TrafficLightPhase thisPhase = _currentPhase;
            _messages.send(std::move(thisPhase));
            t1 = std::chrono::steady_clock::now();

            duration = getRandomDuration(4000, 6000);
        }
    }
}