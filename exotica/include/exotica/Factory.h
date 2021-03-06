/*
 *      Author: Michael Camilleri
 * 
 * Copyright (c) 2016, University Of Edinburgh 
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 
 *  * Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 *  * Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *  * Neither the name of  nor the names of its contributors may be used to 
 *    endorse or promote products derived from this software without specific 
 *    prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#ifndef EXOTICA_OBJECT_FACTORY_H
#define EXOTICA_OBJECT_FACTORY_H

#include <boost/shared_ptr.hpp>
#include <map>
#include <typeinfo>

#include "exotica/Server.h"
#include "exotica/Tools.h"
#include <pluginlib/class_list_macros.h>

/**
 * \brief Generic Factory Macro definition: to be specialised by each new base type which wishes to make use of a factory for instantiation of derived classes
 * @param IDENT   Identifier Type : should be specialised by redefining a macro
 * @param BASE    Base Object type : should be specialised by redefining a macro
 * @param TYPE    The name to identify the class (should be of type IDENT)
 * @param DERIV   The Derived Class type (should inherit from BASE)
 */
#define EXOTICA_REGISTER(BASE, TYPE, DERIV) static exotica::Registrar<BASE> EX_UNIQ(object_registrar_, __LINE__) ("exotica/" TYPE, [] () -> BASE * { return new DERIV(); }, #BASE ); \
    PLUGINLIB_EXPORT_CLASS(DERIV, BASE)

#define EXOTICA_REGISTER_CORE(BASE, TYPE, DERIV) static exotica::Registrar<BASE> EX_UNIQ(object_registrar_, __LINE__) ("exotica/" TYPE, [] () -> BASE * { return new DERIV(); }, #BASE );


namespace exotica
{
  template<typename BO> class Registrar;
  /**
   * \brief Templated Object factory for Default-constructible classes. The Factory is itself a singleton.
   * @param I   The identifier type (typically a string)
   * @param BO  The Base Object type
   */
  template<class BO>
  class Factory: public Object
  {
    public:
      friend class Registrar<BO>;
      /**
       * \brief Singleton implementation: returns a reference to a singleton instance of the instantiated class
       */
      static Factory<BO> & Instance(void)
      {
        static Factory<BO> factory_; //!< Declared static so will only be created once
        return factory_;    //!< At other times, just return the reference to it
      }
      ;

      /**
       * \brief Registers a new derived class type
       * @param type[in]    The name of the class (string): must be a unique identifier
       * @param creator[in] A pointer to the creator function
       */
      void registerType(const std::string & type, BO * (*creator_function)())
      {
        if (type_registry_.find(type) == type_registry_.end()) //!< If it does not already exist
        {
          type_registry_[type] = creator_function;
        }
        else //!< I.e. it exists, then cannot re-create it!
        {
          throw_pretty("Trying to register already existing type '"<<type<<"'!");
        }
      }


      boost::shared_ptr<BO> createInstance(const std::string & type)
      {
          auto it = type_registry_.find(type);  //!< Attempt to find it
          if (it != type_registry_.end())       //!< If exists
          {
             return boost::shared_ptr<BO>(it->second());
          }
          else
          {
            throw_pretty("This factory does not recognize type '"<< type << "' ("+base_type_+")");
          }
      }

      /**
       * \brief Lists the valid implementations which are available and registered
       */
      std::vector<std::string> getDeclaredClasses()
      {
        std::vector<std::string> DeclaredClasses;
        for (auto it = type_registry_.begin(); it != type_registry_.end(); it++)
        {
          DeclaredClasses.push_back(it->first);
        }
        return DeclaredClasses;
      }

      // Depricated
      void createObject(const std::string & type, boost::shared_ptr<BO> & object)
      {
        object = createInstance(type);
      }

      // Depricated
      void createObject(boost::shared_ptr<BO> & object,
          tinyxml2::XMLHandle & handle, const Server_ptr & server)
      {
        if (handle.ToElement())
        {
            std::string type = std::string(handle.ToElement()->Name());
            object = createInstance(type);
              const char * atr = handle.ToElement()->Attribute("name");
              if (atr)
              {
                std::string name = std::string(atr);
                if (name.length() > 0)
                {
                    //const_cast< boost::shared_ptr<BO>& >(object)->object_name_=name;
                    object->object_name_ = name;
                    object->ns_ = name;
                    object->initBase(handle, server);
                }
                else
                {
                  throw_pretty("Object name for object of type '"<< type <<"' was not specified.");
                }
              }
              else
              {
                throw_pretty("Object name for object of type '"<< type <<"' was not specified.");
              }
        }
        else
        {
          throw_pretty("Invalid XML handle");
        }
      }

    private:
      /**
       * \brief Private Constructor
       */
      inline explicit Factory<BO>()
      {
      }

      /** The Map containing the register of the different types of classes **/
      std::map<std::string, BO * (*)()> type_registry_;
      std::string base_type_;
  };

  /**
   * \brief Registration Class for the object type: Also templated:
   * @param I   The Identifier type (typically string)
   * @param BO  The Base object type (required for the sake of the singleton factory)
   */
  template<typename BO>
  class Registrar
  {
    public:
      /**
       * \brief Public Constructor which is only used to register the new task type
       * @param name      The name for the new class type
       * @param creator   The creator function for the DERIVED class type but which returns a pointer to the base-class type!
       */
      Registrar(const std::string & name, BO * (*creator)(), const std::string& base_type)
      {
        Factory<BO>::Instance().base_type_ = base_type;
        Factory<BO>::Instance().registerType(name, creator);
      }
  };
}

#endif
