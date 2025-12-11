#ifndef GAEACTOR_MANAGER_H
#define GAEACTOR_MANAGER_H
#include <QObject>
#include <unordered_map>
#include "head_define.h"

#include "gaeactor_transmit_define.h"
#include "gaeactor_transmit_interface.h"

#include "gaeactor_comm_define.h"

namespace originaldatastoragestd {
    class OriginalDataOutputManager;
};

namespace msg {
    namespace SimParamsInfo {
        class msg_BinaryData;
        class msg_BinaryData_Array;
    };
};

typedef std::function<void (const E_EVENT_MODE& ,const std::vector<EVENT_INFO>&)> eventlist_update_callback;
typedef std::function<void(const TYPE_ULID &, const TYPE_ULID &,const tagPathInfo&)> path_update_callback;
class KlusterWebSocketClient;
class GaeactorManager : public QObject
{
    Q_OBJECT
public:
    static GaeactorManager & getInstance();
    virtual ~GaeactorManager();

    void init();

    void setRunning(bool bRunning);
    const QString& localip() const;

    void setLocalip(const QString &newLocalip);

    const UINT16& hub_websocketport() const;
    void setHub_websocketport(UINT16 newHub_websocketport);

    void set_usinglocalwsaddr(bool newBusinglocalwsaddr);

private:
    explicit GaeactorManager(QObject *parent = nullptr);
    void setRecord(bool newRecord);

    void binary_receive_data_call_back(const BYTE*pdata, const UINT32& ilen);
    void receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE*pdata, const UINT32& ilen, const BYTE*pOriginaldata, const UINT32& iOriginallen);

    void deal_binarydata_callback(const COMM_CHANNEL_INFO &channelinfo, const msg::SimParamsInfo::msg_BinaryData_Array *pdata);
    void calc_trans_speed(const UINT32& iOriginallen);
    void recording_data(const BYTE*pOriginaldata, const UINT32& iOriginallen);
private:

	KlusterWebSocketClient* m_wsclient_;

    QString m_localip;
    UINT16 m_hub_websocketport;
    bool m_businglocalwsaddr;

    COMM_CHANNEL_INFO m_binary_channel;

    originaldatastoragestd::OriginalDataOutputManager *m_pOriginalDataOutputManager_send;

    std::atomic_bool m_record;

    UINT64 m_array_packindex = 0;
    UINT64 m_item_packindex = 0;
};
#endif // GAEACTOR_MANAGER_H
