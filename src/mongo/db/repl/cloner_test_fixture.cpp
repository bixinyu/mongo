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

#include "mongo/platform/basic.h"

#include "mongo/db/clientcursor.h"
#include "mongo/db/repl/cloner_test_fixture.h"
#include "mongo/db/repl/storage_interface.h"
#include "mongo/db/repl/storage_interface_mock.h"
#include "mongo/db/service_context_test_fixture.h"
#include "mongo/dbtests/mock/mock_dbclient_connection.h"
#include "mongo/logger/logger.h"
#include "mongo/unittest/unittest.h"
#include "mongo/util/concurrency/thread_pool.h"

namespace mongo {
namespace repl {

/* static */
BSONObj ClonerTestFixture::createCountResponse(int documentCount) {
    return BSON("n" << documentCount << "ok" << 1);
}

/* static */
BSONObj ClonerTestFixture::createCursorResponse(const std::string& nss, const BSONArray& docs) {
    return BSON("cursor" << BSON("id" << CursorId(0) << "ns" << nss << "firstBatch" << docs) << "ok"
                         << 1);
}

void ClonerTestFixture::setUp() {
    unittest::Test::setUp();
    logger::globalLogDomain()->setMinimumLoggedSeverity(
        logger::LogComponent::kReplicationInitialSync, logger::LogSeverity::Debug(1));
    Client::initThread("ClonerTest");
    ThreadPool::Options options;
    options.minThreads = 1U;
    options.maxThreads = 1U;
    options.onCreateThread = [](StringData threadName) { Client::initThread(threadName); };
    _dbWorkThreadPool = std::make_unique<ThreadPool>(options);
    _dbWorkThreadPool->startup();
    _source = HostAndPort{"local:1234"};
    _mockServer = std::make_unique<MockRemoteDBServer>(_source.toString());
    const bool autoReconnect = true;
    _mockClient = std::unique_ptr<DBClientConnection>(
        new MockDBClientConnection(_mockServer.get(), autoReconnect));
    _sharedData = std::make_unique<InitialSyncSharedData>(
        ServerGlobalParams::FeatureCompatibility::Version::kFullyUpgradedTo44, kInitialRollbackId);
}

void ClonerTestFixture::tearDown() {
    _dbWorkThreadPool.reset();
    Client::releaseCurrent();
    logger::globalLogDomain()->setMinimumLoggedSeverity(
        logger::LogComponent::kReplicationInitialSync, logger::LogSeverity::Debug(0));
    unittest::Test::tearDown();
}

}  // namespace repl
}  // namespace mongo
