#ifndef GAEACTOR_TRANSMIT_WAITSET_H
#define GAEACTOR_TRANSMIT_WAITSET_H

#include "src/local_topic_data.h"
#include <QObject>

#include <thread>
#include "src/local_topic_data.h"
namespace iox {
namespace mepoo
{
/// @brief Helper struct to use as default template parameter when no user-header is used
struct NoUserHeader;
}
namespace popo {

template <typename T, typename H = mepoo::NoUserHeader>
class Subscriber;
}
}
namespace gaeactortransmit
{
class GaeactorTransmitWaitset : public QObject
{
    Q_OBJECT
public:    
    explicit GaeactorTransmitWaitset(QObject *parent = nullptr);
    virtual ~GaeactorTransmitWaitset();
    void attachState(iox::popo::Subscriber<ChannelObject>* data);
private:
    void eventLoop();
    std::thread eventLoopThread;
    std::atomic_bool keepRunning{true};
};
}
#endif // GAEACTOR_TRANSMIT_WAITSET_H
