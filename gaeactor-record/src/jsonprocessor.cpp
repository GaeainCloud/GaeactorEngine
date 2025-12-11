#include "jsonprocessor.h"
#include <iomanip>
#include <iostream>
#include <QJsonObject>
#include <QJsonArray>
#include "configmanager.h"
#include "settingsconfig.h"
#include <QDebug>
#if 0
#define JSONCONS_NO_DEPRECATED
#include "jsoncons/json.hpp"
#include "jsoncons_ext/cbor/cbor.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "simdjson/simdjson.h"
#include "minibson/minibson.hpp"
#include "nlohmann/json.hpp"
//#include "minibson/microbson.hpp"
#endif
#include "gzip.h"

//#define TRACE_STR
#define DATA_EX_LEN (2)

#if 0
bool load_to_string(const char *filename) {
  std::cout << "Loading " << filename << std::endl;
  simdjson::dom::parser parser;
  simdjson::dom::element doc;
  auto error = parser.load(filename).get(doc);
  if (error) { std::cerr << error << std::endl; return false; }
  auto serial1 = simdjson::to_string(doc);
  error = parser.parse(serial1).get(doc);
  if (error) { std::cerr << error << std::endl; return false; }
  auto serial2 = simdjson::to_string(doc);
  bool match = (serial1 == serial2);
  if (match) {
    std::cout << "Parsing to_string and calling to_string again results in the "
                 "same content."
              << std::endl;
  } else {
    std::cout << "The content differs!" << std::endl;
  }
  return match;
}
bool load_minify(const char *filename) {
  std::cout << "Loading " << filename << std::endl;
  simdjson::dom::parser parser;
  simdjson::dom::element doc;
  auto error = parser.load(filename).get(doc);
  if (error) { std::cerr << error << std::endl; return false; }
  auto serial1 = simdjson::minify(doc);
  error = parser.parse(serial1).get(doc);
  if (error) { std::cerr << error << std::endl; return false; }
  auto serial2 = simdjson::minify(doc);
  bool match = (serial1 == serial2);
  if (match) {
    std::cout << "Parsing minify and calling minify again results in the same "
                 "content."
              << std::endl;
  } else {
    std::cout << "The content differs!" << std::endl;
  }
  return match;
}



QJsonObject qjson_to_minibsondoc(const minibson::document& minibson_doc)
{
    QJsonObject json_dst;
    minibson::element_list::const_iterator itor = minibson_doc.begin();
    while(itor != minibson_doc.end())
    {
        auto key = itor->first;
        auto val = itor->second;

        switch (val->get_node_code()) {
        case minibson::double_node:
            {
                double valret = reinterpret_cast<const minibson::type_converter<double>::node_class*>(val)->get_value();
                json_dst.insert(QString::fromStdString(key),QJsonValue(valret));
            }
            break;
        case minibson::string_node:
            {
                std::string valret = reinterpret_cast<const minibson::type_converter<std::string>::node_class*>(val)->get_value();
                json_dst.insert(QString::fromStdString(key),QJsonValue(QString::fromStdString(valret)));

            }
            break;

        case minibson::document_node:
            {
                minibson::document *valret = reinterpret_cast<minibson::document*>(val);
                json_dst.insert(QString::fromStdString(key),qjson_to_minibsondoc(*valret));
            }
            break;

        case minibson::binary_node:
            {
                double valret = reinterpret_cast<const minibson::type_converter<double>::node_class*>(val)->get_value();
                json_dst.insert(QString::fromStdString(key),QJsonValue(valret));
            }
            break;
        case minibson::boolean_node:
            {
                bool valret = reinterpret_cast<const minibson::type_converter<bool>::node_class*>(val)->get_value();
                json_dst.insert(QString::fromStdString(key),QJsonValue(valret));
            }
            break;
        case minibson::int32_node:
            {
                int valret = reinterpret_cast<const minibson::type_converter<int>::node_class*>(val)->get_value();
                json_dst.insert(QString::fromStdString(key),QJsonValue(valret));
            }
            break;
        case minibson::int64_node:
            {
                long long int valret = reinterpret_cast<const minibson::type_converter<long long int>::node_class*>(val)->get_value();
                json_dst.insert(QString::fromStdString(key),QJsonValue(valret));
            }
            break;
        case minibson::null_node:
        case minibson::unknown_node:
            {
                json_dst.insert(QString::fromStdString(key),QJsonValue());
            }
            break;
        }
        itor++;
    }
    return json_dst;
}

minibson::document qjson_to_minibsondoc(const QJsonObject& qjson_src)
{
    minibson::document minibson_doc;
    for(auto key:qjson_src.keys())
    {
        std::string key_std_str = key.toStdString();
        auto jsonval = qjson_src.value(key);
        switch (jsonval.type()) {
        case QJsonValue::Null:
            {
                minibson_doc.set(key_std_str);
            }
            break;
        case QJsonValue::Bool:
            {
                minibson_doc.set(key_std_str,jsonval.toBool());
            }
            break;
        case QJsonValue::Double:
            {
                minibson_doc.set(key_std_str,jsonval.toDouble());
            }
            break;
        case QJsonValue::String:
            {
                minibson_doc.set(key_std_str,jsonval.toString().toStdString());
            }
            break;
        case QJsonValue::Array:
            {
                QJsonArray jsarray = jsonval.toArray();
                for(auto jsarrayItem:jsarray)
                {

                }
            }
            break;
        case QJsonValue::Object:
            {
                minibson_doc.set(key_std_str,qjson_to_minibsondoc(jsonval.toObject()));
            }
            break;
        case QJsonValue::Undefined:break;
        }
    }
    return minibson_doc;
}
#endif
#if 0
QString JsonProcessor::simdjsonDeserialized(const QByteArray &jsonStr)
{
    const char * src = jsonStr.toStdString().c_str();
    auto serial2 = simdjson::to_string(src);
    return QString();
}

QByteArray JsonProcessor::simdjsonSerialized(const QJsonObject &jsonObj)
{
    QString jsonStr = AosKernelCommon::json_object_to_string(jsonObj, true);
    std::string src = jsonStr.toStdString();

    simdjson::dom::parser parser;
    simdjson::dom::element doc;
    auto error = parser.parse(src).get(doc);
        if (error) { std::cerr << error << std::endl; return QByteArray(); }
    auto dst = simdjson::minify(doc);
    QByteArray minibytearray(dst.c_str());
    return minibytearray;
}
#endif

#if 0
QByteArray JsonProcessor::minibsonSerialized(const QJsonObject &jsonObj)
{
    minibson::document d2 = qjson_to_minibsondoc(jsonObj);
    size_t size2 = d2.get_serialized_size();
    QByteArray byteArray;
    byteArray.resize(size2);
    d2.serialize(byteArray.data(), size2);
    return byteArray;
}

QJsonObject JsonProcessor::minibsonDeserialized(const QByteArray &byteArray)
{
    minibson::document d3(byteArray.data(), byteArray.size());
    QJsonObject tmp = qjson_to_minibsondoc(d3);
    return tmp;
}


std::vector<uint8_t> QByteArrayToVector(const QByteArray &byteArray)
{
    std::vector<uint8_t> buffer;
    buffer.resize(byteArray.size());
    memcpy(buffer.data(),byteArray.data(),byteArray.size());
    return buffer;
}

QByteArray VectorToQByteArray(const std::vector<uint8_t> &buffer)
{
    QByteArray byteArray;
    byteArray.resize(buffer.size());
    memcpy(byteArray.data(),buffer.data(),buffer.size());
    return byteArray;
}

std::vector<uint8_t> JsonProcessor::jsonconsSerialized(const std::string &jsonStr)
{
    jsoncons::json j = jsoncons::json::parse(jsonStr);
    std::vector<uint8_t> buffer;
    jsoncons::cbor::encode_cbor(j, buffer);
    return buffer;
}

std::string JsonProcessor::jsonconsDeserialized(const std::vector<uint8_t> &byteArray)
{
    jsoncons::json jTmp = jsoncons::cbor::decode_cbor<jsoncons::json>(byteArray);

    return jTmp.to_string();
}

std::vector<uint8_t> JsonProcessor::nlohmannSerialized(const std::string &jsonStr)
{
    nlohmann::json j = nlohmann::json::parse(jsonStr);

//    auto barray = nlohmann::json::to_bson(j);
    auto barray = nlohmann::json::to_cbor(j);
//    auto barray = nlohmann::json::to_msgpack(j);
//    auto barray = nlohmann::json::to_ubjson(j);
    return barray;
}

std::string JsonProcessor::nlohmannDeserialized(const std::vector<uint8_t> &byteArray)
{
//    nlohmann::json j = nlohmann::json::from_bson(byteArray);
    nlohmann::json j = nlohmann::json::from_cbor(byteArray);
//    nlohmann::json j = nlohmann::json::from_msgpack(byteArray);
//    nlohmann::json j = nlohmann::json::from_ubjson(byteArray);
    return j.dump();
}
#include <QDateTime>
#endif

JsonProcessor::JsonProcessor(QObject *parent)
{

}

JsonProcessor::~JsonProcessor()
{

}

QByteArray JsonProcessor::gzip(unsigned char * pOriginBuffer,unsigned int pOriginBufferLen)
{
    QByteArray bytmpBuf;
    uLong pCompressedTmpBufferLen = pOriginBufferLen*DATA_EX_LEN;
    bytmpBuf.resize(pCompressedTmpBufferLen);
    unsigned char * pCompressedBuffer = reinterpret_cast<unsigned char *>(bytmpBuf.data());
    int pCompressedBufferLen = gzcompress(pOriginBuffer,
               pOriginBufferLen,
               pCompressedBuffer,
               pCompressedTmpBufferLen);    
#if 0
    QByteArray bytmp;
    if(pCompressedBufferLen!=-1)
    {
        bytmp.resize(pCompressedBufferLen);
        memcpy(bytmp.data(),pCompressedBuffer,sizeof(unsigned char)*(uLong)pCompressedBufferLen);
    }
    return bytmp;
#else
    if(pCompressedBufferLen!=-1)
    {
        bytmpBuf.resize(pCompressedBufferLen);
    }
    else
    {
        bytmpBuf.resize(0);
    }
    return bytmpBuf;
#endif
}

QByteArray JsonProcessor::gzip(const QByteArray &by)
{
    unsigned char * pOriginBuffer = (unsigned char *)(const_cast<char *>(by.data()));
    return gzip(pOriginBuffer,(uLong)by.size());
}

QByteArray JsonProcessor::gzip(const std::vector<uint8_t> & src_data)
{
    unsigned char * pOriginBuffer = const_cast<unsigned char *>( src_data.data());
    return gzip(pOriginBuffer,(uLong)src_data.size());
}

QByteArray JsonProcessor::serialized(const QJsonObject &jsonObj)
{
    QString jsonStr = SettingsConfig::getInstance().json_object_to_string(jsonObj, true);
    return serialized(jsonStr);
}

QByteArray JsonProcessor::gunzip(const QByteArray &byteArray)
{    
    if(byteArray.isEmpty() || byteArray.size() < sizeof(uLong)*DATA_EX_LEN)
    {
        return QByteArray();
    }
    uLong pCompressedBufferLen  =0;
    uLong pUncompressedTmpBufferLen = 0;
    memcpy(&pUncompressedTmpBufferLen,byteArray.data(),sizeof(uLong));
    memcpy(&pCompressedBufferLen,byteArray.data()+sizeof(uLong),sizeof(uLong));
    if((pCompressedBufferLen +sizeof(uLong)*DATA_EX_LEN) != byteArray.size())
    {
        return QByteArray();
    }
    unsigned char * pOriginBuffer = (unsigned char *)(const_cast<char *>(byteArray.data()+sizeof(uLong)*DATA_EX_LEN));

    QByteArray bytmpBuf;
    bytmpBuf.resize(pUncompressedTmpBufferLen);
    unsigned char * pUncompressedBuffer = reinterpret_cast<unsigned char *>(bytmpBuf.data());

    int pUnCompressedBufferLen = gzdecompress(pOriginBuffer,
                                 pCompressedBufferLen,
                                 pUncompressedBuffer,
                                 pUncompressedTmpBufferLen);
#if 0
    QByteArray bytmp;
    if(pUnCompressedBufferLen!=-1)
    {
        bytmp.resize(pUnCompressedBufferLen);
        memcpy(bytmp.data(),bytmpBuf.data(),sizeof(unsigned char)*(uLong)pUnCompressedBufferLen);
    }
    return bytmp;
#else
    if(pUnCompressedBufferLen!=-1)
    {
        bytmpBuf.resize(pUnCompressedBufferLen);
    }
    else
    {
        bytmpBuf.resize(0);
    }
    return bytmpBuf;
#endif
}

QByteArray JsonProcessor::gunzip(const unsigned char * const pData, unsigned int iDataLen)
{
    if(nullptr  == pData || iDataLen < sizeof(uLong)*DATA_EX_LEN)
    {
        return QByteArray();
    }
    uLong pCompressedBufferLen  =0;
    uLong pUncompressedTmpBufferLen = 0;
    memcpy(&pUncompressedTmpBufferLen,pData,sizeof(uLong));
    memcpy(&pCompressedBufferLen,pData+sizeof(uLong),sizeof(uLong));
    if((pCompressedBufferLen + sizeof(uLong)*DATA_EX_LEN) != iDataLen)
    {
        return QByteArray();
    }
    unsigned char * pOriginBuffer = const_cast<unsigned char *>(pData+sizeof(uLong)*DATA_EX_LEN);

    QByteArray bytmpBuf;
    bytmpBuf.resize(pUncompressedTmpBufferLen);
    unsigned char * pUncompressedBuffer = reinterpret_cast<unsigned char *>(bytmpBuf.data());

    int pUnCompressedBufferLen = gzdecompress(pOriginBuffer,
                                 pCompressedBufferLen,
                                 pUncompressedBuffer,
                                 pUncompressedTmpBufferLen);
#if 0
    QByteArray bytmp;
    if(pUnCompressedBufferLen!=-1)
    {
        bytmp.resize(pUnCompressedBufferLen);
        memcpy(bytmp.data(),bytmpBuf.data(),sizeof(unsigned char)*(uLong)pUnCompressedBufferLen);
    }
    return bytmp;
#else
    if(pUnCompressedBufferLen!=-1)
    {
        bytmpBuf.resize(pUnCompressedBufferLen);
    }
    else
    {
        bytmpBuf.resize(0);
    }
    return bytmpBuf;
#endif
}

QJsonObject JsonProcessor::deserialized(const QByteArray &byteArray)
{
    auto UnCompressedByte = deserialized_from_bytearray(byteArray);
    QJsonObject jsonObj = SettingsConfig::getInstance().string_to_json_object(QString(UnCompressedByte));
    return jsonObj;
}

QByteArray JsonProcessor::serialized(const QString &savestr)
{
    QByteArray by = savestr.toUtf8();
    return serialized((const unsigned char * const)by.data(), by.length());
}

QByteArray JsonProcessor::serialized(const std::string &savestr)
{
    return serialized((const unsigned char *)savestr.c_str(),savestr.length());
}

QByteArray JsonProcessor::serialized(const QByteArray &byteArray)
{
    return serialized((const unsigned char * const ) byteArray.data(), byteArray.size());
}

QByteArray JsonProcessor::serialized(const unsigned char * const pData, unsigned int iDataLen)
{
#ifdef TRACE_STR
    qint64 beginjsonconsTime = QDateTime::currentMSecsSinceEpoch();
#endif
    unsigned char * pOriginBuffer = const_cast<unsigned char *>(pData);
    uLong uncompressedsize = iDataLen;
    auto gzip2 = gzip(pOriginBuffer,iDataLen);
    uLong compressedsize = (uLong)gzip2.size();

    QByteArray retdata;
    retdata.resize(sizeof(uLong)*DATA_EX_LEN+compressedsize);
    memcpy(retdata.data(),&uncompressedsize,sizeof(uLong));
    memcpy(retdata.data()+sizeof(uLong),&compressedsize,sizeof(uLong));
    memcpy(retdata.data()+sizeof(uLong)*DATA_EX_LEN,gzip2.data(),(size_t)gzip2.size());
#ifdef TRACE_STR
    qint64 gzipTime2 = QDateTime::currentMSecsSinceEpoch();
    QString traceStr =QString("serialized :uncompressed len:%1 gzip1:compressed len:%2 compressibility:%3 compressed time:%4 ").arg(uncompressedsize)
            .arg(sizeof(uLong)*DATA_EX_LEN+compressedsize)
            .arg((float)compressedsize/(float)uncompressedsize*100)
            .arg(gzipTime2 - beginjsonconsTime);
    qDebug()<<traceStr;
#endif
    return retdata;
}



QByteArray JsonProcessor::deserialized(const unsigned char * const pData, unsigned int iDataLen)
{
#ifdef TRACE_STR
    qint64 beginjsonconsTime = QDateTime::currentMSecsSinceEpoch();
#endif
    auto UnCompressedByte = gunzip(pData,iDataLen);
#ifdef TRACE_STR
    qint64 endnlohmannTime = QDateTime::currentMSecsSinceEpoch();
    QString traceStr = QString("deserialized: compressed len:%1 uncompressed len:%2 compressed time:%3").arg(iDataLen).arg(UnCompressedByte.size()).arg(endnlohmannTime - beginjsonconsTime);
    qDebug()<<traceStr;
#endif
    return UnCompressedByte;
}

QByteArray JsonProcessor::deserialized_from_bytearray(const QByteArray &byteArray)
{
    return deserialized((const unsigned char * const)byteArray.data(), byteArray.size());
}

QJsonObject JsonProcessor::deserialized_to_jsonobj(const unsigned char * const pData, unsigned int iDataLen)
{
    auto UnCompressedByte = deserialized(pData,iDataLen);
    QJsonObject jsonObj = SettingsConfig::getInstance().string_to_json_object(QString(UnCompressedByte));
    return jsonObj;
}

