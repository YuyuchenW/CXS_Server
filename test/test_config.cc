#include "../code/config.hpp"
#include "../code/log.h"
#include <yaml-cpp/yaml.h>
CXS::ConfigVar<int>::ptr g_int_value_config = CXS::Config::Lookup("system.port", (int)8080, "system port");

//CXS::ConfigVar<float>::ptr g_int_valuex_config = CXS::Config::Lookup("system.port", (float)82080, "system port");
CXS::ConfigVar<float>::ptr g_float_value_config = CXS::Config::Lookup("system.value", (float)3984.23f, "system value");
CXS::ConfigVar<std::vector<int>>::ptr g_int_vector_value_config = CXS::Config::Lookup("system.int_vector", std::vector<int>(2, 2), "system int vector");

CXS::ConfigVar<std::list<int>>::ptr g_int_list_value_config = CXS::Config::Lookup("system.int_list", std::list<int>(2, 2), "system int list");
CXS::ConfigVar<std::set<int>>::ptr g_int_set_value_config = CXS::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system set set");
CXS::ConfigVar<std::unordered_set<int>>::ptr g_int_unordered_set_value_config = CXS::Config::Lookup("system.int_unordered_set", std::unordered_set<int>{1, 2}, "system unordered_set");
CXS::ConfigVar<std::map<std::string, int>>::ptr g_int_map_value_config = CXS::Config::Lookup("system.int_map", std::map<std::string, int>{{"cxs", 123}}, "system map");
CXS::ConfigVar<std::unordered_map<std::string, int>>::ptr g_int_unordered_map_value_config = CXS::Config::Lookup("system.int_unordered_map", std::unordered_map<std::string, int>{{"cxs", 213}}, "system unordered map");

void print_yaml(const YAML::Node &node, int level)
{
    if (node.IsScalar())
    {
        CXS_LOG_INFO(CXS_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if (node.IsNull())
    {
        CXS_LOG_INFO(CXS_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL "
                                     << " - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            CXS_LOG_INFO(CXS_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            CXS_LOG_INFO(CXS_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}
void test_yaml()
{
    YAML::Node root = YAML::LoadFile("/cxs/bin/conf/test.yml");
    print_yaml(root, 0);
    // CXS_LOG_INFO(CXS_LOG_ROOT()) <<root;
}

void test_config()
{
    CXS_LOG_INFO(CXS_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    CXS_LOG_INFO(CXS_LOG_ROOT()) << "before: " << g_float_value_config->toString();
#define XX(g_var, name, prefix)                                                          \
    {                                                                                    \
        auto v = g_var->getValue();                                                      \
        for (auto &i : v)                                                                \
        {                                                                                \
            CXS_LOG_INFO(CXS_LOG_ROOT()) << #prefix " " #name " :" << i;                 \
        }                                                                                \
        CXS_LOG_INFO(CXS_LOG_ROOT()) << #prefix " " #name " yaml:" << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix)                                                        \
    {                                                                                    \
        auto v = g_var->getValue();                                                      \
        for (auto &i : v)                                                                \
        {                                                                                \
            CXS_LOG_INFO(CXS_LOG_ROOT()) << #prefix " " #name " :"                       \
                                         << "{" << i.first                               \
                                         << " - " << i.second << "}";                    \
        }                                                                                \
        CXS_LOG_INFO(CXS_LOG_ROOT()) << #prefix " " #name " yaml:" << g_var->toString(); \
    }

    XX(g_int_vector_value_config, int_vector, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_unordered_set_value_config, unordered_set, before);
    XX_M(g_int_map_value_config, int_map, before);
    XX_M(g_int_unordered_map_value_config, int_unordered_map, before);
    g_int_value_config->addListener( [](const int &old_value, const int &new_value)
                                    { CXS_LOG_ERROR(CXS_LOG_ROOT()) << " old int value = " << old_value << " and  new value = "
                                                                   << new_value; });
    YAML::Node root = YAML::LoadFile("/cxs/bin/conf/test.yml");
    CXS::Config::LoadFromYaml(root);

    CXS_LOG_INFO(CXS_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    CXS_LOG_INFO(CXS_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vector_value_config, int_vector, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_unordered_set_value_config, unordered_set, after);
    XX_M(g_int_map_value_config, int_map, after);
    XX_M(g_int_unordered_map_value_config, int_unordered_map, after);
}

void test_log()
{
    CXS::Logger::ptr logSys = CXS_LOG_NAME("system");
    CXS_LOG_FATAL(logSys) << "hello system";
    //std::cout<< CXS::LoggerMgr::GetInstance() -> toYamlString() <<std::endl;
    YAML::Node root = YAML::LoadFile("/cxs/bin/conf/log.yml");
    CXS::Config::LoadFromYaml(root);
    //std::cout<<"-----------------------------------------"<<std::endl;
    //std::cout<< CXS::LoggerMgr::GetInstance() -> toYamlString() <<std::endl;
    CXS_LOG_FATAL(logSys) << "hello system";
    CXS_LOG_FATAL(CXS_LOG_ROOT()) << "hello root u";

    logSys->setFormatter("%d - %m%n");
    CXS_LOG_FATAL(logSys) << "hello system";


}

int main(int argc, char const *argv[])
{
    // CXS_LOG_INFO(CXS_LOG_ROOT()) << g_int_value_config->getValue();
    // CXS_LOG_INFO(CXS_LOG_ROOT()) << g_float_value_config->toString();
    //  test_yaml();
    //test_config();
    test_log();
    CXS::Config::Visit([](CXS::ConfigVarBase::ptr var){
        CXS_LOG_INFO(CXS_LOG_ROOT()) << "name = " << var->getName()
                                    <<" description = " << var ->getDescription()
                                    <<" typename = " << var -> getTypeName()
                                    <<" value = " <<var->toString();

    });
    return 0;
}
