TEMPLATE = subdirs

win32 {

SUBDIRS = gaeactor \
        gaeactor-agent-cores \
        gaeactor-agent-sensors \
        gaeactor-interactions \
        gaeactor-auditions \
        gaeactor-event-engine \
        gaeactor-environment \
        gaeactor-environment-ex \
        gaeactor-viewer \
        gaeactor-transmit \
        gaeactor-test \
        gaeactor-display \
        gaeactor-record \
        gaeactor-tools \
        gaeactor-comm \
        TierAirlineAirPortSimulation \
        gaeactor-benchmark \
        gaeactor-engine \
        gaeactor-presentation-test \
#        gaeactor-presentation-test2 \
        gaeactor-presentation-test3 \
        gaeactor-presentation-test4 \
        gaeactor-presentation-test5 \
        gaeactor-presentation-test6 \
        gaeactor-presentation-test7 \
        gaeactor-presentation-test8 \
        gaeactor-comm-test

gaeactor-agent-cores.depends = gaeactor-environment gaeactor-event-engine
gaeactor-agent-sensors.depends = gaeactor-environment gaeactor-event-engine
gaeactor-interactions.depends = gaeactor-environment
gaeactor-auditions.depends = gaeactor-environment
gaeactor-event-engine.depends = gaeactor-environment
gaeactor-viewer.depends = gaeactor-environment gaeactor-agent-cores gaeactor-agent-sensors gaeactor-interactions gaeactor-auditions gaeactor-event-engine
gaeactor-test.depends = gaeactor-transmit
gaeactor-comm-test.depends = gaeactor-transmit
gaeactor-benchmark.depends = gaeactor-transmit  gaeactor gaeactor-engine
gaeactor-engine.depends = gaeactor-transmit  gaeactor
gaeactor-display.depends = gaeactor-transmit
gaeactor-record.depends = gaeactor-transmit
gaeactor-transmit.depends = gaeactor-comm
gaeactor.depends = gaeactor-environment gaeactor-agent-cores gaeactor-agent-sensors gaeactor-interactions gaeactor-auditions gaeactor-event-engine gaeactor-transmit
TierAirlineAirPortSimulation.depends = gaeactor-transmit
gaeactor-presentation-test7.depends = gaeactor-environment-ex
}

unix {

SUBDIRS = gaeactor \
        gaeactor-agent-cores \
        gaeactor-agent-sensors \
        gaeactor-interactions \
        gaeactor-auditions \
        gaeactor-event-engine \
        gaeactor-environment \
        gaeactor-environment-ex \
        gaeactor-viewer \
        gaeactor-transmit \
        gaeactor-record \
        gaeactor-comm \
        gaeactor-test \
        TierAirlineAirPortSimulation \
        gaeactor-comm-test \
#        gaeactor-benchmark \
#        gaeactor-engine


gaeactor-agent-cores.depends = gaeactor-environment gaeactor-event-engine
gaeactor-agent-sensors.depends = gaeactor-environment gaeactor-event-engine
gaeactor-interactions.depends = gaeactor-environment
gaeactor-auditions.depends = gaeactor-environment
gaeactor-event-engine.depends = gaeactor-environment
gaeactor-viewer.depends = gaeactor-environment gaeactor-agent-cores gaeactor-agent-sensors gaeactor-interactions gaeactor-auditions gaeactor-event-engine
gaeactor-test.depends = gaeactor-transmit
gaeactor-comm-test.depends = gaeactor-transmit
#gaeactor-benchmark.depends = gaeactor-transmit  gaeactor gaeactor-engine
#gaeactor-engine.depends = gaeactor-transmit  gaeactor
gaeactor-record.depends = gaeactor-transmit
gaeactor-transmit.depends = gaeactor-comm
gaeactor.depends = gaeactor-environment gaeactor-agent-cores gaeactor-agent-sensors gaeactor-interactions gaeactor-auditions gaeactor-event-engine gaeactor-transmit
TierAirlineAirPortSimulation.depends = gaeactor-transmit
}
