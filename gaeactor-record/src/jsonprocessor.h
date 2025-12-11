#ifndef JSON_PROCESSOR_H_
#define JSON_PROCESSOR_H_

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QJsonObject>

class JsonProcessor : public QObject
{
    Q_OBJECT
public:
    JsonProcessor(QObject* parent = nullptr);
    virtual ~JsonProcessor();
#if 0
    static QString simdjsonDeserialized(const QByteArray& json);
    static QByteArray simdjsonSerialized(const QJsonObject& jsonObj);
#endif

    static QByteArray serialized(const QJsonObject& jsonObj);


    static QByteArray serialized(const QString & savestr );
    static QByteArray serialized(const std::string & savestr );
    static QByteArray serialized(const unsigned char *const pData, unsigned int iDataLen);
    static QByteArray serialized(const QByteArray &byteArray);
    static QJsonObject deserialized(const QByteArray &byteArray);
    static QByteArray deserialized(const unsigned char *const pData, unsigned int iDataLen);
    static QByteArray deserialized_from_bytearray(const QByteArray &byteArray);
    static QJsonObject deserialized_to_jsonobj(const unsigned char *const pData, unsigned int iDataLen);
private:
    static QByteArray gunzip(const QByteArray &byteArray);
    static QByteArray gunzip(const unsigned char *const pData, unsigned int iDataLen);
    static QByteArray gzip(unsigned char * pOriginBuffer,unsigned int pOriginBufferLen);
    static QByteArray gzip(const QByteArray &by);
    static QByteArray gzip(const std::vector<uint8_t> & src_data);
#if 0
private:
    static QByteArray minibsonSerialized(const QJsonObject& jsonObj);
    static QJsonObject minibsonDeserialized(const QByteArray &byteArray);
    static std::vector<uint8_t> jsonconsSerialized(const std::string& jsonObj);
    static std::string jsonconsDeserialized(const std::vector<uint8_t> &byteArray);

    static std::vector<uint8_t> nlohmannSerialized(const std::string &jsonObj);
    static std::string nlohmannDeserialized(const std::vector<uint8_t> &byteArray);
#endif
};


#endif // aosk_link_mavlink_h
