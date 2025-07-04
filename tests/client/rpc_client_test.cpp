#include <gtest/gtest.h>
#include <muduo/base/Thread.h>
#include "client/rpc_client.h"
#include "utils/time_util.h"

TEST(RpcClientTest, InitializationTest) 
{
    RustCinder::RpcClient* rpcClient = nullptr;
    muduo::Thread t([&rpcClient]() {
        rpcClient = new RustCinder::RpcClient();
        rpcClient->init();
        ASSERT_TRUE(rpcClient->getEventLoop() != nullptr) << "Event loop should be initialized.";

        rpcClient->start();
        delete rpcClient;
    });
    t.start();

    sleep(1);
    ASSERT_TRUE(rpcClient != nullptr) << "RpcClient should be created.";
    rpcClient->stop();
    rpcClient = nullptr;
    t.join();
}

TEST(RpcClientTest, LoginTest) 
{
    RustCinder::RpcClient* rpcClient = nullptr;
    muduo::Thread t([&rpcClient](){
        rpcClient = new RustCinder::RpcClient();
        rpcClient->init();
        ASSERT_TRUE(rpcClient->getEventLoop() != nullptr) << "Event loop should be initialized.";

        rpcClient->start();
        delete rpcClient;

    });
    t.start();

    // Wait for the login response
    sleep(1);

    std::string account = "test_user";
    std::string password = "test_password";

    // Simulate a login request
    rpcClient->getLoginServiceStub()->login(account, password);

    ASSERT_TRUE(rpcClient != nullptr) << "RpcClient should be created.";

    rpcClient->stop();
    rpcClient = nullptr;
    t.join();
}

TEST(RpcClientTest, SyncServerTimeTest)
{
    RustCinder::RpcClient* rpcClient = nullptr;
    muduo::Thread t([&rpcClient](){
        rpcClient = new RustCinder::RpcClient();
        rpcClient->init();
        ASSERT_TRUE(rpcClient->getEventLoop() != nullptr) << "Event loop should be initialized.";

        rpcClient->start();
        delete rpcClient;

    });
    t.start();

    // Wait for the login response
    sleep(6);

    ASSERT_TRUE(RustCinder::TimeUtil::lastSyncTs != 0) << "Server time should be synced.";

    rpcClient->stop();
    rpcClient = nullptr;
    t.join();
}

TEST(RpcClientTest, PingTest)
{
    RustCinder::RpcClient* rpcClient = nullptr;
    muduo::Thread t([&rpcClient](){
        rpcClient = new RustCinder::RpcClient();
        rpcClient->init();
        ASSERT_TRUE(rpcClient->getEventLoop() != nullptr) << "Event loop should be initialized.";

        rpcClient->start();
        delete rpcClient;
    });
    t.start();

    // Wait for the ping response
    sleep(2);
    ASSERT_TRUE(RustCinder::TimeUtil::lastPingTs != 0) << "Ping should be successful.";
    rpcClient->stop();
    rpcClient = nullptr;
    t.join(); 
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}