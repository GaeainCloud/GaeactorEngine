win32: {
INCLUDEPATH += $$PWD/../../3rd/win64/boost-1.84.0/include/boost-1_84

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/boost-1.84.0/lib/release 
}

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/boost-1.84.0/lib/debug
}
}


unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/boost-1.84.0/debug/include
#        LIBS += -L$$PWD/../../3rd/lnx/boost-1.84.0/static_debug/lib \
#        -lboost_fiber \
#        -lboost_math_c99f \
#        -lboost_regex \
#        -lboost_timer \
#        -lboost_atomic \
#        -lboost_filesystem \
#        -lboost_math_c99l \
#        -lboost_serialization \
#        -lboost_type_erasure \
#        -lboost_chrono \
#        -lboost_graph \
#        -lboost_math_tr1 \
#        -lboost_stacktrace_addr2line \
#        -lboost_unit_test_framework \
#        -lboost_container \
#        -lboost_iostreams \
#        -lboost_math_tr1f\
#        -lboost_stacktrace_backtrace \
#        -lboost_url \
#        -lboost_context \
#        -lboost_json\
#        -lboost_math_tr1l\
#        -lboost_stacktrace_basic \
#        -lboost_wave \
#        -lboost_contract \
#        -lboost_locale \
#        -lboost_nowide \
#        -lboost_stacktrace_noop \
#        -lboost_wserialization \
#        -lboost_coroutine \
#        -lboost_log\
#        -lboost_prg_exec_monitor \
#        -lboost_system \
#        -lboost_date_time \
#        -lboost_log_setup \
#        -lboost_program_options \
#        -lboost_test_exec_monitor \
#        -lboost_exception \
#        -lboost_math_c99 \
#        -lboost_random\
#        -lboost_thread

    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/boost-1.84.0/release/include
#        LIBS += -L$$PWD/../../3rd/lnx/boost-1.84.0/static_release/lib \
#        -lboost_fiber \
#        -lboost_math_c99f \
#        -lboost_regex \
#        -lboost_timer \
#        -lboost_atomic \
#        -lboost_filesystem \
#        -lboost_math_c99l \
#        -lboost_serialization \
#        -lboost_type_erasure \
#        -lboost_chrono \
#        -lboost_graph \
#        -lboost_math_tr1 \
#        -lboost_stacktrace_addr2line \
#        -lboost_unit_test_framework \
#        -lboost_container \
#        -lboost_iostreams \
#        -lboost_math_tr1f\
#        -lboost_stacktrace_backtrace \
#        -lboost_url \
#        -lboost_context \
#        -lboost_json\
#        -lboost_math_tr1l\
#        -lboost_stacktrace_basic \
#        -lboost_wave \
#        -lboost_contract \
#        -lboost_locale \
#        -lboost_nowide \
#        -lboost_stacktrace_noop \
#        -lboost_wserialization \
#        -lboost_coroutine \
#        -lboost_log\
#        -lboost_prg_exec_monitor \
#        -lboost_system \
#        -lboost_date_time \
#        -lboost_log_setup \
#        -lboost_program_options \
#        -lboost_test_exec_monitor \
#        -lboost_exception \
#        -lboost_math_c99 \
#        -lboost_random\
#        -lboost_thread


    }
}

