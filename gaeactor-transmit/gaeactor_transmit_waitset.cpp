#include "gaeactor_transmit_waitset.h"
#include <QDebug>

#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include <iostream>
namespace gaeactortransmit
{

iox::cxx::optional<iox::popo::WaitSet<>> waitset;
GaeactorTransmitWaitset::GaeactorTransmitWaitset(QObject *parent)
    :QObject(parent)
{
    keepRunning.store(true);
    eventLoopThread = std::thread(&GaeactorTransmitWaitset::eventLoop, this);
    waitset.emplace();
}

GaeactorTransmitWaitset::~GaeactorTransmitWaitset()
{
    keepRunning.store(false);
    eventLoopThread.join();
    waitset.reset();
}

void GaeactorTransmitWaitset::attachState(iox::popo::Subscriber<ChannelObject> *data)
{
    // attach subscriber to waitset
    waitset->attachState(*data, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
        std::cerr << "failed to attach subscriber" << std::endl;
        std::exit(EXIT_FAILURE);
    });
    //! [create waitset]
}


void GaeactorTransmitWaitset::eventLoop()
{
    while (keepRunning)
    {
        auto notificationVector = waitset->wait();
        //for (auto& notification : notificationVector)
        {
//            if (notification->getNotificationId() == ACTIVATE_ID)
//            {
//                // reset MyTriggerClass instance state
//                notification->getOrigin<MyTriggerClass>()->reset(MyTriggerClassStates::IS_ACTIVATED);
//                // call the callback attached to the trigger
//                (*notification)();
//            }
//            else if (notification->getNotificationId() == ACTION_ID)
//            {
//                // reset is not required since we attached an notification here. we will be notified once
//                (*notification)();
//            }

//            // We woke up and hence there must be at least one sample. When the sigHandler has called
//            // markForDestruction the notificationVector is empty otherwise we know which subscriber received samples
//            // since we only attached one.
//            // Best practice is to always acquire the notificationVector and iterate over all elements and then react
//            // accordingly. When this is not done and more elements are attached to the WaitSet it can cause
//            // problems since we either miss events or handle events for objects which never occurred.
//            if (notification->doesOriginateFrom(m_pChannelOperateSubscriber))
//            {
//                // Consume a sample
//                (*m_pChannelOperateSubscriber).take()
//                    .and_then([&](auto& sample) {auto  & channelobj = *sample; dealChannelObject(channelobj); std::cout << " got value: " << std::endl; })
//                    .or_else([](auto& reason) {
//                        std::cout << "got no data, return code: " << static_cast<uint64_t>(reason) << std::endl;
//                    });
//                // We could consume all samples but do not need to.
//                // If there is more than one sample we will wake up again since the state of the subscriber is still
//                // iox::popo::SubscriberState::HAS_DATA in this case.
//            }
        }
        std::cout << "wait something test"<<std::endl;
    }
}


}
