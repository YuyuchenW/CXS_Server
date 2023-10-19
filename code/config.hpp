#ifndef __CXS_CONFIG_H__
#define __CXS_CONFIG_H__

#include <functional>
#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <list>
#include <map>
#include <vector>
#include <unordered_set>
#include "log.h"
#include "thread.h"
namespace CXS
{
    // From type F to target type T
    template <class F, class T>
    class LexicalCast
    {
    public:
        T operator()(const F &v)
        {

            return boost::lexical_cast<T>(v);
        }
    };

    // 对vector偏特化
    template <class T>
    class LexicalCast<std::string,
                      std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <class T>
    class LexicalCast<std::vector<T>,
                      std::string>
    {
    public:
        std::string operator()(const std::vector<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // 对list偏特化
    template <class T>
    class LexicalCast<std::string,
                      std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::list<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template <class T>
    class LexicalCast<std::list<T>,
                      std::string>
    {
    public:
        std::string operator()(const std::list<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // 对set偏特化
    template <class T>
    class LexicalCast<std::string, std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::set<T> st;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                st.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return st;
        }
    };

    template <class T>
    class LexicalCast<std::set<T>, std::string>
    {
    public:
        std::string operator()(const std::set<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
    // 对unordered_set偏特化
    template <class T>
    class LexicalCast<std::string, std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_set<int> st;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); ++i)
            {
                ss.str("");
                ss << node[i];
                st.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return st;
        }
    };

    template <class T>
    class LexicalCast<std::unordered_set<T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T> &v)
        {
            YAML::Node node(YAML::NodeType::Sequence);
            for (auto &i : v)
            {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // 对map偏特化
    template <class T>
    class LexicalCast<std::string, std::map<std::string, T>>
    {
    public:
        std::map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, T> mp;
            std::stringstream ss;
            for (auto i = node.begin(); i != node.end(); ++i)
            {
                ss.str("");
                ss << i->second;
                mp.insert({i->first.Scalar(), LexicalCast<std::string, T>()(ss.str())});
            }
            return mp;
        }
    };

    template <class T>
    class LexicalCast<std::map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::map<std::string, T> &v)
        {
            YAML::Node node(YAML::NodeType::Map);
            for (auto &i : v)
            {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // 对unordered_map偏特化
    template <class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>>
    {
    public:
        std::unordered_map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_map<std::string, T> mp;
            std::stringstream ss;
            for (auto i = node.begin(); i != node.end(); ++i)
            {
                ss.str("");
                ss << i->second;
                mp.insert({i->first.Scalar(), LexicalCast<std::string, T>()(ss.str())});
            }
            return mp;
        }
    };

    template <class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string, T> &v)
        {
            YAML::Node node(YAML::NodeType::Map);
            for (auto &i : v)
            {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string &name, const std::string &description = "")
            : m_name(name), m_description(description)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {}

        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        virtual bool fromString(const std::string &val) = 0;
        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    public:
        typedef RWMutex RWMutexType;
        typedef std::shared_ptr<ConfigVar> ptr;
        typedef std::function<void(const T &old_value, const T &new_value)> on_change_callback;

        ConfigVar(const std::string &name,
                  const T &default_value,
                  const std::string description = "") : ConfigVarBase(name, description), m_value(default_value) {}

        std::string toString() override
        {
            try
            {
                // return boost::lexical_cast<std::string>(m_value);
                RWMutexType::ReadLock lock(m_mutex);
                return ToStr()(m_value);
            }
            catch (const std::exception &e)
            {
                CXS_LOG_ERROR(CXS_LOG_ROOT()) << "ConfigVar::toString exception" << e.what()
                                              << "convert: " << typeid(m_value).name() << " to string";
            }
            return "";
        }

        bool fromString(const std::string &val) override
        {
            try
            {
                setValue(FromStr()(val));
                // m_value = boost::lexical_cast<T>(val);
                return true;
            }
            catch (const std::exception &e)
            {
                CXS_LOG_ERROR(CXS_LOG_ROOT()) << "ConfigVar::fromString exception" << e.what()
                                              << "convert: string to " << typeid(m_value).name();
            }
            return false;
        }
        const T getValue() 
        {
            RWMutexType::ReadLock lock(m_mutex);
            return m_value;
        }
        void setValue(const T &value)
        {
            {
                RWMutexType::ReadLock lock(m_mutex);

                if (value == m_value)
                {
                    return;
                }
                for (auto &it : m_cbs)
                {
                    it.second(m_value, value);
                }
            }
            RWMutexType::WriteLock lock(m_mutex);
            m_value = value;
        }
        std::string getTypeName() const override { return typeid(T).name(); }
        uint64_t addListener(on_change_callback cb)
        {
            static uint64_t s_fun_id = 0;
            RWMutexType::WriteLock lock(m_mutex);
            ++s_fun_id;
            m_cbs[s_fun_id] = cb;
            return s_fun_id;
        }
        void delListener(uint64_t key)
        {
            RWMutexType::WriteLock lock(m_mutex);
            m_cbs.erase(key);
        }

        void clearListener()
        {
            RWMutexType::WriteLock lock(m_mutex);

            m_cbs.clear();
        }
        on_change_callback getListener(uint64_t key)
        {
            RWMutexType::ReadLock lock(m_mutex);
            auto it = m_cbs.find(key);
            return it == m_cbs.end() ? nullptr : it->second;
        }

    private:
        RWMutexType m_mutex;
        T m_value;
        std::unordered_map<uint64_t, on_change_callback> m_cbs;
    };

    class Config
    {
    public:
        typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
        typedef RWMutex RWMutexType;
        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name)
        {
            RWMutexType::ReadLock lock(GetMutex());

            auto item = GetDatas().find(name);
            if (item == GetDatas().end())
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T>>(item->second);
        }

        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name,
                                                 const T &default_value, const std::string &description = "")
        {
            // auto tem = Lookup<T>(name);

            RWMutexType::WriteLock lock(GetMutex());
            auto item = GetDatas().find(name);
            if (item != GetDatas().end())
            {
                auto temp = std::dynamic_pointer_cast<ConfigVar<T>>(item->second);
                if (temp)
                {
                    CXS_LOG_INFO(CXS_LOG_ROOT()) << "Look up name =" << name << "exist";
                    return temp;
                }
                else
                {
                    CXS_LOG_ERROR(CXS_LOG_ROOT()) << "Look up name = " << name << "but type different! not " << typeid(T).name() << " real type = " << item->second->getTypeName()
                                                  << " and name = " << item->second->getName();
                }
            }
            // if (tem)
            // {
            //     CXS_LOG_INFO(CXS_LOG_ROOT()) << "Look up name =" << name << "exist";
            //     return tem;
            // }
            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._12345678") != std::string::npos)
            {
                CXS_LOG_ERROR(CXS_LOG_ROOT()) << "Look up name invalid" << name;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr var(new ConfigVar<T>(name, default_value, description));
            GetDatas()[name] = var;
            return var;
        };
        static void LoadFromYaml(const YAML::Node &root);
        static ConfigVarBase::ptr LookupBase(const std::string &name);

        static void Visit(std::function<void(ConfigVarBase::ptr)> cb); 
    private:
        static ConfigVarMap &GetDatas()
        {
            static ConfigVarMap s_datas;
            return s_datas;
        }

        static RWMutexType& GetMutex()
        {
            static RWMutexType s_mutex;
            return s_mutex;
        }
    };
}

#endif