set(source_files
    model/ns3/zash-server.cc
    model/ns3/zash-device-enforcer.cc
    model/zash-enums.cc
    model/zash-markov.cc
    model/zash-models.cc
    helper/zash-utils.cc
    model/audit/zash-audit.cc
    model/action/zash-attack.cc
    model/action/zash-alteration.cc
    model/behavior/zash-configuration.cc
    model/behavior/zash-notification.cc
    model/collection/zash-data.cc
    model/collection/zash-device.cc
    model/decision/zash-activity.cc
    model/decision/zash-authorization.cc
    model/decision/zash-conflict.cc
    model/decision/zash-context.cc
    model/decision/zash-ontology.cc
)

set(header_files
    model/ns3/zash-server.h
    model/ns3/zash-device-enforcer.h
    model/zash-enums.h
    model/zash-markov.h
    model/zash-models.h
    helper/zash-utils.h
    model/audit/zash-audit.h
    model/action/zash-attack.h
    model/action/zash-alteration.h
    model/behavior/zash-configuration.h
    model/behavior/zash-notification.h
    model/collection/zash-data.h
    model/collection/zash-device.h
    model/decision/zash-activity.h
    model/decision/zash-authorization.h
    model/decision/zash-conflict.h
    model/decision/zash-context.h
    model/decision/zash-ontology.h
)

build_lib(
  LIBNAME zash
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libnetwork}
    ${libinternet}
    ${libapplications}
  TEST_SOURCES
    test/zash-test-suite.cc
)
