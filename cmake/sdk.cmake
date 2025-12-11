include(${CMAKE_DIR}/sdk_crashservice.cmake)
include(${CMAKE_DIR}/sdk_gaeactor.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_agent_cores.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_agent_sensors.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_auditions.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_engine.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_environment.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_environment_ex.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_event_engine.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_interactions.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_presentation.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_heads.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_genginecore.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)
include(${CMAKE_DIR}/sdk_phos_core.cmake)
include(${CMAKE_DIR}/sdk_triangle.cmake)

include(${CMAKE_DIR}/sdk_dynielcore.cmake)
include(${CMAKE_DIR}/sdk_dynielmssn.cmake)
include(${CMAKE_DIR}/sdk_gaeacomm.cmake)
include(${CMAKE_DIR}/sdk_gaeaobject.cmake)
include(${CMAKE_DIR}/sdk_gaeavariable.cmake)
include(${CMAKE_DIR}/sdk_filters.cmake)
include(${CMAKE_DIR}/sdk_freqdist.cmake)
include(${CMAKE_DIR}/sdk_demservice_standalone.cmake)
include(${CMAKE_DIR}/sdk_rcs.cmake)
include(${CMAKE_DIR}/sdk_commcluster.cmake)
include(${CMAKE_DIR}/sdk_propagator.cmake)
include(${CMAKE_DIR}/sdk_gaingraph.cmake)
include(${CMAKE_DIR}/sdk_instagent.cmake)
include(${CMAKE_DIR}/sdk_cognitivecluster.cmake)
include(${CMAKE_DIR}/sdk_sensorcluster.cmake)
include(${CMAKE_DIR}/sdk_heightevent.cmake)
include(${CMAKE_DIR}/sdk_radionoise.cmake)
include(${CMAKE_DIR}/sdk_commclusterservice.cmake)
include(${CMAKE_DIR}/sdk_commphysicslayerservice.cmake)
include(${CMAKE_DIR}/sdk_cognitiveclusterservice.cmake)
include(${CMAKE_DIR}/sdk_sensorclusterservice.cmake)
include(${CMAKE_DIR}/sdk_doctrineclusterservice.cmake)





set(SDK_LIBRARIES
    ${sdk_crashservice}
    ${sdk_gaeactor}
    ${sdk_gaeactor_agent_cores}
    ${sdk_gaeactor_agent_sensors}
    ${sdk_gaeactor_auditions}
    ${sdk_gaeactor_comm}
    # ${sdk_gaeactor_engine}
    ${sdk_gaeactor_environment}
    # ${sdk_gaeactor_environment_ex}
    ${sdk_gaeactor_event_engine}
    ${sdk_gaeactor_interactions}
    # ${sdk_gaeactor_presentation}
    ${sdk_gaeactor_transmit}
    ${sdk_gaeactor_utils}
    ${sdk_gaeactor_utils_heads}
    ${sdk_gaeactor_utils_location}
    ${sdk_genginecore}
    ${sdk_loggingservice}
    ${sdk_phos_core}
    ${sdk_triangle}
    
    #${sdk_dynielcore}
    #${sdk_dynielmssn}    
    #${sdk_gaeacomm}    
    #${sdk_gaeaobject}    
    #${sdk_gaeavariable}

    #${sdk_filters}
    #${sdk_freqdist}
    #${sdk_rcs}
    # ${sdk_commcluster}
    #${sdk_propagator}
    #${sdk_gaingraph}
    #${sdk_instagent}
    #${sdk_cognitivecluster}
    #${sdk_sensorcluster}
    #${sdk_heightevent}
    #${sdk_radionoise}
    #${sdk_commclusterservice}
    #${sdk_commphysicslayerservice}
    #${sdk_cognitiveclusterservice}
    #${sdk_sensorclusterservice}
    #${sdk_doctrineclusterservice}
    )
