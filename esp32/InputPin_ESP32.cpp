#include "InputPin_ESP32.h"
#include <thread>
#include <iostream>



using namespace std;
using namespace LIO;

InputPin_ESP32::InputPin_ESP32(gpio_num_t inputPinNo):threadExitRequest{false},threadState{listenerThreadState::stopped},gpioNo{inputPinNo}{
    gpio_set_direction(gpioNo,gpio_mode_t::GPIO_MODE_INPUT);
    gpio_intr_disable(gpioNo);
    static bool justOnce=false;
    if(!justOnce){
        gpio_install_isr_service(0);
        justOnce=true;
    }
}

void InputPin_ESP32::SetTriggerEdge(LIO::InputPin::TriggerEdge edge){
    switch (edge) {
    case InputPin::TriggerEdge::None:
        gpio_set_intr_type(gpioNo,gpio_int_type_t::GPIO_INTR_DISABLE);
        break;
    case InputPin::TriggerEdge::Both:
        gpio_set_intr_type(gpioNo,gpio_int_type_t::GPIO_INTR_ANYEDGE);
        break;
    case InputPin::TriggerEdge::Rising:
        gpio_set_intr_type(gpioNo,gpio_int_type_t::GPIO_INTR_POSEDGE);
        break;
    case InputPin::TriggerEdge::Falling:
        gpio_set_intr_type(gpioNo,gpio_int_type_t::GPIO_INTR_NEGEDGE);
        break;
    }
}

void InputPin_ESP32::SetPullUpDown(LIO::InputPin::PullUpDown pud){
    switch (pud) {
    case InputPin::PullUpDown::HiZ:
        gpio_pullup_dis(gpioNo);
        gpio_pulldown_dis(gpioNo);
        break;
    case InputPin::PullUpDown::PullUp:
        gpio_pulldown_dis(gpioNo);
        gpio_pullup_en(gpioNo);
        break;
    case InputPin::PullUpDown::PullDown:
        gpio_pullup_dis(gpioNo);
        gpio_pulldown_en(gpioNo);
        break;

    }
}

void InputPin_ESP32::StartListening(){
    if(threadState==listenerThreadState::stopped){
//        std::promise<void> p;
//        exitPerformed=p.get_future();
//        thread t(bind(&InputPin_ESP32::listenerThreadRunnable,this,placeholders::_1),move(p));
        threadState=listenerThreadState::started;
//        t.detach();
        gpio_isr_handler_add(gpioNo,InputPin_ESP32::ISR,reinterpret_cast<void*>(this));
        cout<<"thread started"<<endl;
        gpio_intr_enable(gpioNo);
    }
    else
        cout<<"Listener thread already started for input pin "<<GetPinNo()<<"!"<<endl;
}

void InputPin_ESP32::StopListening(){
    if(threadState==listenerThreadState::started){
//        threadExitRequest=true;
//        signal.signal();
//        while(exitPerformed.wait_for(10ms)!=future_status::ready){
//            signal.signal();
//        }
        threadState=listenerThreadState::stopped;
        gpio_intr_disable(gpioNo);
        gpio_isr_handler_remove(gpioNo);
    }
}
void InputPin_ESP32::listenerThreadRunnable(std::promise<void> &&threadExitPromise)
{
    while(!threadExitRequest){
        signal.wait();
        if(threadExitRequest)
            break;
        callEventCallback();
        this_thread::sleep_for(10ms);
    }
    threadExitPromise.set_value_at_thread_exit();
}

void InputPin_ESP32::ISR(void *context)
{
    cout<<"executed"<<endl;
//    InputPin_ESP32* instance=reinterpret_cast<InputPin_ESP32*>(context);
//    instance->signal.signal();
}

//void IRAM_ATTR InputPin_ESP32::ISR(void * context)
//{
//    InputPin_ESP32* instance=reinterpret_cast<InputPin_ESP32*>(context);
//    instance->signal.signal();
//}
bool InputPin_ESP32::Read()
{
    return gpio_get_level(gpioNo);
}

uint32_t InputPin_ESP32::GetPinNo(){
    return static_cast<uint32_t>(gpioNo);
}
