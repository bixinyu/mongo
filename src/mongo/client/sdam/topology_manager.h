/**
 *    Copyright (C) 2019-present MongoDB, Inc.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the Server Side Public License, version 1,
 *    as published by MongoDB, Inc.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Server Side Public License for more details.
 *
 *    You should have received a copy of the Server Side Public License
 *    along with this program. If not, see
 *    <http://www.mongodb.com/licensing/server-side-public-license>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the Server Side Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */
#pragma once
#include <memory>

#include "mongo/client/sdam/sdam_datatypes.h"
#include "mongo/client/sdam/topology_description.h"
#include "mongo/client/sdam/topology_state_machine.h"

namespace mongo::sdam {
/**
 * This class serves as the public interface to the functionality described in the Service Discovery
 * and Monitoring spec:
 *   https://github.com/mongodb/specifications/blob/master/source/server-discovery-and-monitoring/server-discovery-and-monitoring.rst
 */
class TopologyManager {
    TopologyManager() = delete;
    TopologyManager(const TopologyManager&) = delete;

public:
    TopologyManager(SdamConfiguration config, ClockSource* clockSource);

    /**
     * This function atomically:
     *   1. Clones the current TopologyDescription
     *   2. Executes the state machine logic given the cloned TopologyDescription and provided
     * IsMasterOutcome (containing the new ServerDescription).
     *   3. Installs the cloned (and possibly modified) TopologyDescription as the current one.
     *
     * Multiple threads may call this function concurrently. However, the manager will process the
     * IsMasterOutcomes serially, as required by:
     *   https://github.com/mongodb/specifications/blob/master/source/server-discovery-and-monitoring/server-discovery-and-monitoring.rst#process-one-ismaster-outcome-at-a-time
     */
    void onServerDescription(const IsMasterOutcome& isMasterOutcome);

    /**
     * Get the current TopologyDescription. This is safe to call from multiple threads.
     */
    const TopologyDescriptionPtr getTopologyDescription() const;

private:
    mutable mongo::Mutex _mutex = mongo::Mutex(StringData("TopologyManager"));
    const SdamConfiguration _config;
    ClockSource* _clockSource;
    std::shared_ptr<TopologyDescription> _topologyDescription;
    std::unique_ptr<TopologyStateMachine> _topologyStateMachine;
};
}  // namespace mongo::sdam
