
#include <google/protobuf/descriptor.h>

#include "service/service_manager.h"

namespace RustCinder::Service
{

    ServiceManager::~ServiceManager()
    {
        // Clean up all registered services
        for (auto& servicePair : m_services)
        {
            if (servicePair.second)
            {
                delete servicePair.second; // Clean up each service
            }
        }
        m_services.clear(); // Clear the service map
    }

    std::string ServiceManager::getServiceName(google::protobuf::Service* service)
    {
        if (service)
        {
            const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
            return desc->full_name();
        }
        return "";
    }

    void ServiceManager::registerService(google::protobuf::Service* service)
    {
        if (service)
        {
            std::string serviceName = getServiceName(service);
            if (!serviceName.empty())
            {
                m_services[serviceName] = service;
            }
        }
    }

    void ServiceManager::unregisterService(const std::string& serviceName)
    {
        auto it = m_services.find(serviceName);
        if (it != m_services.end())
        {
            m_services.erase(it);
        }
    }

    google::protobuf::Service* ServiceManager::getService(const std::string& serviceName) const
    {
        auto it = m_services.find(serviceName);
        if (it != m_services.end())
        {
            return it->second;
        }
        return nullptr; // Service not found
    }
}