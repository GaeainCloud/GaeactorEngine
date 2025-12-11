#ifndef AGENTDTO_H_
#define AGENTDTO_H_
#include <functional>
#include <QJsonObject>
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/////////////////////////////////////////////////////////////////////////////////////////////////////

class simulationRangeDto : public oatpp::DTO {

    DTO_INIT(simulationRangeDto, DTO)
    DTO_FIELD(String, timestamp_min);
    DTO_FIELD(String, timestamp_max);
    DTO_FIELD(String, timestamp_cur);
    DTO_FIELD(String, time_min);
    DTO_FIELD(String, time_max);
    DTO_FIELD(String, time_cur);
    DTO_FIELD(String, simname);
};

class simulationReviewRangeDto : public oatpp::DTO {

    DTO_INIT(simulationReviewRangeDto, DTO)
    DTO_FIELD(String, timestamp_min);
    DTO_FIELD(String, timestamp_max);
    DTO_FIELD(String, time_min);
    DTO_FIELD(String, time_max);
    DTO_FIELD(String, timestamp_review_min);
    DTO_FIELD(String, time_review_min);
    DTO_FIELD(String, timestamp_review_max);
    DTO_FIELD(String, time_review_max);
    DTO_FIELD(String, timestamp_review_cur);
    DTO_FIELD(String, time_review_cur);
    DTO_FIELD(String, simname);
};

class simulationReviewDataDto : public oatpp::DTO {

    DTO_INIT(simulationReviewDataDto, DTO)
    DTO_FIELD(String, reviewdata);
    DTO_FIELD(String, operate);
};


class simulationReviewItemDto : public oatpp::DTO {

    DTO_INIT(simulationReviewItemDto, DTO)
    DTO_FIELD(String, reviewdata);
    DTO_FIELD(UInt64, size);
    DTO_FIELD(String, date);
    DTO_FIELD(String, simid);
    DTO_FIELD(String, titlename);
    DTO_FIELD(Int64, iTimeStampStart);
    DTO_FIELD(Int64, iTimeStampEnd);
    DTO_FIELD(String, iTimeStampStartStr);
    DTO_FIELD(String, iTimeStampEndStr);
    DTO_FIELD(String, iTimeStampStr);
    DTO_FIELD(UInt64, iValidDataTotalLen);
    DTO_FIELD(UInt64, iFrames);
};


class simulationReviewReturnDto : public oatpp::DTO {

    DTO_INIT(simulationReviewReturnDto, DTO)
    DTO_FIELD(Vector<Object<simulationReviewItemDto>>, reviewdatas);
};




class flightInfoDto : public oatpp::DTO {

    DTO_INIT(flightInfoDto, DTO)
    DTO_FIELD(String, flightid);
    DTO_FIELD(String, target_parkingpoint);
    DTO_FIELD(String, target_runway);
    DTO_FIELD(String, alloc_parkingpoint);
    DTO_FIELD(String, alloc_runway);
//    DTO_FIELD(String, date);
//    DTO_FIELD(String, filghtNumber);
//    DTO_FIELD(String, depArrFlag);
//    DTO_FIELD(String, planeNum);
//    DTO_FIELD(String, planeType);
//    DTO_FIELD(String, flightClass);
//    DTO_FIELD(String, flightLeg);
//    DTO_FIELD(String, flightStartPlace);
//    DTO_FIELD(String, flightEndPlace);
//    DTO_FIELD(String, planDateTimeTakeOff);
//    DTO_FIELD(String, planDateTimeLanding);
//    DTO_FIELD(String, seat);
//    DTO_FIELD(String, terminal);
//    DTO_FIELD(String, runway);
};


class flightQueryReturnDto : public oatpp::DTO {

    DTO_INIT(flightQueryReturnDto, DTO)
    DTO_FIELD(Vector<Object<flightInfoDto>>, flightInfos);
};


class flightInfoReviewDto : public oatpp::DTO {

    DTO_INIT(flightInfoReviewDto, DTO)
    DTO_FIELD(String, flightid);
    DTO_FIELD(String, target_parkingpoint);
    DTO_FIELD(String, target_runway);
    DTO_FIELD(String, alloc_parkingpoint);
    DTO_FIELD(String, alloc_runway);
    DTO_FIELD(String, date);
    DTO_FIELD(String, filghtNumber);
    DTO_FIELD(String, depArrFlag);
    DTO_FIELD(String, planeNum);
    DTO_FIELD(String, planeType);
    DTO_FIELD(String, flightClass);
    DTO_FIELD(String, flightLeg);
    DTO_FIELD(String, flightStartPlace);
    DTO_FIELD(String, flightEndPlace);
    DTO_FIELD(String, planDateTimeTakeOff);
    DTO_FIELD(String, planDateTimeLanding);
    DTO_FIELD(String, seat);
    DTO_FIELD(String, terminal);
    DTO_FIELD(String, runway);
};


class flightReviewQueryReturnDto : public oatpp::DTO {

    DTO_INIT(flightReviewQueryReturnDto, DTO)
    DTO_FIELD(Vector<Object<flightInfoReviewDto>>, flightInfos);
};

class flightQueryRunningDto : public oatpp::DTO {

    DTO_INIT(flightQueryRunningDto, DTO)
    DTO_FIELD(Vector<String>, flightsId);
};


class simulationCtrlDto : public oatpp::DTO {

    DTO_INIT(simulationCtrlDto, DTO)
    DTO_FIELD(String, ctrltype);
    DTO_FIELD(String, ctrlparam);
};


class simulationCtrlReturnDto : public oatpp::DTO {

    DTO_INIT(simulationCtrlReturnDto, DTO)
    DTO_FIELD(String, ctrltype);
    DTO_FIELD(String, ctrlparam);
    DTO_FIELD(String, ctrlparam_status);
};


class simulationDataDto : public oatpp::DTO {

    DTO_INIT(simulationDataDto, DTO)
    DTO_FIELD(String, airportcode);
    DTO_FIELD(Vector<String>, arrrunway);
    DTO_FIELD(Vector<String>, deprunway);
    DTO_FIELD(Vector<String>, allowrunway);
    DTO_FIELD(String, simname);
    DTO_FIELD(String, url);
};

class recordCtrlDto : public oatpp::DTO {

    DTO_INIT(recordCtrlDto, DTO)
    DTO_FIELD(Boolean, ctrl);
};


class simparamDto : public oatpp::DTO {

    DTO_INIT(simparamDto, DTO)
    DTO_FIELD(Int32, step_interval);
    DTO_FIELD(Float64, step_dt);
    DTO_FIELD(Float64, step_freq);
    DTO_FIELD(Float64, one_second_sim_step_second);

    DTO_FIELD(Float64, review_speed);
    DTO_FIELD(String, review_status);
};


#include OATPP_CODEGEN_END(DTO)

enum E_DATA_TYPE
{
    E_DATA_TYPE_SIM_DATA,
    E_DATA_TYPE_SIM_CTRL,
    E_DATA_TYPE_RECORD_CTRL,
    E_DATA_TYPE_SIM_REVIEW_DATA,
    E_DATA_TYPE_SIM_REVIEW_CTRL,
};
typedef std::function<bool (E_DATA_TYPE, const QJsonObject &)> http_receive_callback;


#endif /* AGENTDTO_H_ */
