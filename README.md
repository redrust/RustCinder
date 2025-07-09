# RustCinder：自建学习库
## 编译流程
1. 安装xmake
```
curl -fsSL https://xmake.io/shget.text | bash
```
2. 开启debug模式
```
xmake f -m debug
```
3. 编译项目
```
xmake build
```
4. 运行测试 

其中tests文件下全部文件名都是独立可运行的测试模块，下面是使用样例
```
xmake run rpc_client_test
```
5. 使用gdb调试
```
xmake run -d rpc_client_test
```
## 安装本地库，可以不安装，xmake已经处理好包管理
1. 安装protobuf
```
sudo apt-get install protobuf-compiler libprotobuf-dev
```
2. 安装muduo
```
1. 安装cmake
sudo apt-get install cmake
2. 安装boost库
sudo apt-get install libboost-dev libboost-test-dev
3. 下载muduo库
git clone https://github.com/chenshuo/muduo.git
4. 注释muduo的unit_test
cmakelists里的这一行 option(MUDUO_BUILD_EXAMPLES "Build Muduo examples" ON) 注释掉
5. 编译
./build.sh
6. 安装
./build.sh install
7. 拷贝库文件
cd ~/build/release-install-cpp11
mv include/muduo/ /usr/include
mv lib/* /usr/local/lib
```
3. 安装gtest
```
git clone https://github.com/google/googletest.git
cd googletest
mkdir build
cd build
cmake ..
make && make install
```
4. 安装yaml
```
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build && cd build
cmake .. && make
```
## 已实现功能
- rpc_client：暂定的客户端，后续需要看看怎么接入到商业引擎里混合编译，目前仅测试使用。
- gateway：荷载未验证客户端连接，同时进行心跳检测等服务。
## 待实现模块
- 内存池
- 协程池
- 服务注册发现中心
- 替换muduo日志库
- 替换muduo网络库
- 反射库