#ifndef _SERVICE_MANAGER_H_
#define _SERVICE_MANAGER_H_

#include <map>

#include <google/protobuf/service.h>

#include "common/noncopyable.h"
#include "common/nonmoveable.h"

namespace RustCinder::Service
{
    class ServiceManager : public NonCopyable, public NonMoveable
    {
    public:
        ServiceManager() = default;
        ~ServiceManager();

        static std::string getServiceName(google::protobuf::Service* service);
        void registerService(google::protobuf::Service* service);
        void unregisterService(const std::string& serviceName);

        const std::map<std::string, google::protobuf::Service*>& getServices() const { return m_services; }
        google::protobuf::Service* getService(const std::string& serviceName) const;
    private:
        std::map<std::string, google::protobuf::Service*> m_services;
    };
}
#endif // _SERVICE_MANAGER_H_