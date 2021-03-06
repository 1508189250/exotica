#ifndef PROPERTY_H
#define PROPERTY_H

#include <exotica/Tools/Printable.h>
#include <exotica/Tools/Exception.h>
#include <exotica/Tools/Conversions.h>
#include <memory>
#include <map>
#include <vector>
#include <typeinfo>
#include <boost/any.hpp>
#include <initializer_list>

namespace exotica
{

class Property
{
public:
    boost::any get() const;
    template<typename C> void set(const C val);
    Property(std::string prop_name);
    Property(std::string prop_name, bool isRequired);
    Property(std::string prop_name, bool isRequired, boost::any val);
    Property(std::initializer_list<boost::any> val);
    bool isRequired() const;
    bool isSet() const;
    bool isStringType() const;
    bool isInitializerVectorType() const;
    std::string getName() const;
    std::string getType() const;

private:
    boost::any value;
    bool required;
    std::string name;
};

class Initializer
{
public:
    Initializer();
    Initializer(std::string name_);
    Initializer(std::string name_, std::map<std::string,boost::any> properties_);
    std::string getName() const;
    void setName(std::string name_);
    void addProperty(const Property& prop);
    boost::any getProperty(std::string name_) const;
    bool hasProperty(std::string name_) const;


    std::map<std::string, Property> properties;
    std::string name;
};

class InitializerBase
{
public:
    virtual void check(const Initializer& other) const = 0;
    virtual Initializer getTemplate() const = 0;
};

class InstantiableBase
{
public:
    virtual Initializer getInitializerTemplate() = 0;
    virtual void InstantiateInternal(const Initializer& init) = 0;
    virtual void InstantiateBase(const Initializer& init) {}
};

template<class C>
class Instantiable : public virtual InstantiableBase
{
public:

    virtual void InstantiateInternal(const Initializer& init)
    {
        InstantiateBase(init);
        C tmp(init);
        tmp.check(init);
        Instantiate(tmp);
    }

    virtual Initializer getInitializerTemplate()
    {
        return C().getTemplate();
    }

    virtual void Instantiate(C& init) = 0;
};

}

#endif // PROPERTY_H
