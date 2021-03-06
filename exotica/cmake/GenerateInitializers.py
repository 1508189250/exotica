#!/usr/bin/env python
from __future__ import print_function
import sys
import os

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def eprint(msg):
    sys.stderr.write(msg+'\n')
    sys.exit(2)

def ConstructorArgumentList(Data):
    ret=""
    for d in Data:
        if d.has_key('Required'):
            ret+=" "+d['Type']+" "+d['Name']+"_"+DefaultArgumentValue(d)+","
    return ret[0:-1]

def ConstructorList(Data):
    ret=""
    for d in Data:
        if d.has_key('Required'):
            ret+=",\n        "+d['Name']+"("+d['Name']+"_) "
    return ret

def DefaultValue(Data):
    if Data['Value']==None:
        return ""
    elif Data['Value']=='{}':
        return ""
    else:
        return Data['Value']

def DefaultArgumentValue(Data):
    if Data['Value']==None:
        return ""
    elif Data['Value']=='{}':
        return "={}"
    else:
        return " = "+Data['Value']

def IsRequired(Data):
    if Data['Required']:
        return "true"
    else:
        return "false"

def DefaultConstructorList(Data):
    ret=""
    for d in Data:
        if d.has_key('Required') and not d['Required']:
            ret+=",\n        "+d['Name']+"("+DefaultValue(d)+") "
    return ret

def NeedsDefaultConstructor(Data):
    for d in Data:
        if d['Required']:
            return True
    return False

def Declaration(Data):
    if Data.has_key('Required'):
      return "    "+Data['Type']+" "+Data['Name']+";\n"
    else:
      return ""

def Parser(Type):
    parser=""
    if Type=='std::string':
        return "boost::any_cast<"+Type+">(prop.get())"
    elif Type=='exotica::Initializer' or Type=='Initializer':
        return "prop.isInitializerVectorType()?boost::any_cast<std::vector<exotica::Initializer>>(prop.get()).at(0):boost::any_cast<exotica::Initializer>(prop.get())"
    elif Type=='std::vector<Initializer>' or Type=='std::vector<exotica::Initializer>':
        return "boost::any_cast<std::vector<exotica::Initializer>>(prop.get())"
    elif Type=='Eigen::VectorXd':
        parser= "parseVector"
    elif Type=='bool':
        parser= "parseBool"
    elif Type=='double':
        parser= "parseDouble"
    elif Type=='int':
        parser= "parseInt"
    elif Type=='std::vector<std::string>':
        parser= "parseList"
    elif Type=='std::vector<int>':
        parser= "parseIntList"
    elif Type=='std::vector<bool>':
        parser= "parseBoolList"
    else:
        eprint("Unknown data type '"+Type+"'")
        sys.exit(2)

    return "prop.isStringType()?"+parser+"(boost::any_cast<std::string>(prop.get())):boost::any_cast<"+Type+">(prop.get())"


def Copy(Data):
    if Data.has_key('Required'):
      return "        ret.properties.emplace(\""+Data['Name']+"\", Property(\""+Data['Name']+"\", "+IsRequired(Data)+", boost::any("+Data['Name']+")));\n"
    else:
      return ""

def Add(Data):
    if Data.has_key('Required'):
      return "        if (other.hasProperty(\""+Data['Name']+"\")) {const Property& prop=other.properties.at(\""+Data['Name']+"\"); if(prop.isSet()) "+Data['Name']+" = "+Parser(Data['Type'])+";}\n"
    else:
      return ""

def Check(Data, Name):
    if Data.has_key('Required') and Data['Required']:
      return "        if(!other.hasProperty(\""+Data['Name']+"\") || !other.properties.at(\""+Data['Name']+"\").isSet()) throw_pretty(\"Initializer "+Name+" requires property "+Data['Name']+" to be set!\");\n"
    else:
      return ""



def Construct(Namespace, ClassName, Data,Include):
    CalssNameOrig=ClassName[0:-11]
    ret="""// This file was automatically generated. Do not edit this file!
#ifndef INITIALIZER_"""+Namespace+"_"+ClassName+"""_H
#define INITIALIZER_"""+Namespace+"_"+ClassName+"""_H

#include "exotica/Property.h"
"""
    for i in Include:
        ret+="#include <"+i+".h>\n"
    ret+="""

namespace """ +Namespace+ """
{

class """+ClassName+" : public InitializerBase"+"""
{
public:
    static std::string getContainerName() {return """+"\""+Namespace+"/"+CalssNameOrig+"\""+ """ ;}

    """
    if NeedsDefaultConstructor(Data):
        ret=ret+ClassName+"() : InitializerBase()"+DefaultConstructorList(Data)+"""
    {
    }

    """
    ret=ret+ClassName+"("+ConstructorArgumentList(Data)+") : InitializerBase()"+ConstructorList(Data)+"""
    {
    }

    """+ClassName+"""(const Initializer& other) : """+ClassName+"""()
    {
"""
    for d in Data:
        ret+=Add(d)
    ret+="""
    }

    virtual Initializer getTemplate() const
    {
        return (Initializer)"""+ClassName+"""();
    }

    virtual void check(const Initializer& other) const
    {
"""
    for d in Data:
        ret+=Check(d,ClassName)
    ret+="""
    }

    operator Initializer()
    {
        Initializer ret(getContainerName());
"""
    for d in Data:
        ret+=Copy(d)
    ret+="""

        return ret;
    }

"""
    for d in Data:
        ret+=Declaration(d)
    ret+="};"+"""

}\n#endif"""
    return ret

def ParseLine(line, ln, fn):
    last = line.find(";")
    if last>=0:
        line=line[0:last].strip()
    else:
        last = line.find("//")
        if last>=0:
            line = line[0:last].strip()
        else:
            line=line.strip()

    if len(line)==0:
        return None

    if line.startswith("include"):
        return {'Include':line[7:].strip().strip(">").strip("<").strip('"'), 'Code':line.strip()}
    if line.startswith("extend"):
        return {'Extends':line[6:].strip().strip(">").strip("<").strip('"'), 'Code':line.strip()}

    if last==-1:
        eprint("Can't find ';' in '"+fn+"', on line " + `ln`)
        sys.exit(2)

    required = True
    if line.startswith("Required"):
        required = True
    elif line.startswith("Optional"):
        required = False
    else:
        eprint("Can't parse 'Required/Optional' tag in '"+fn+"', on line " + `ln`)
        sys.exit(2)

    value = None
    type = ""
    name = ""
    if required==False:
        eq = line.find("=")
        if eq==-1:
            eq=last;
            value = '{}'
        else:
            value = line[eq+1:last]
        nameStart=line[0:eq].strip().rfind(" ")
        name = line[nameStart:eq].strip()
        type = line[9:nameStart].strip()
    else:
        nameStart=line[0:last].strip().rfind(" ")
        name = line[nameStart:last].strip()
        type = line[9:nameStart].strip()

    return {'Required':required, 'Type':type, 'Name':name, 'Value':value}

def ParseFile(filename):
    with open(filename) as f:
        lines = f.readlines()
    Data=[]
    Include=[]
    Extends=[]
    i=0
    optionalOnly=False
    for l in lines:
        i=i+1
        d=ParseLine(l,i,filename)
        if d!=None:
            if d.has_key('Required'):
                if d['Required']==False:
                    optionalOnly=True
                else:
                    if optionalOnly:
                        eprint("Required properties have to come before Optional ones, in '"+filename+"', on line " + `i`)
                        sys.exit(2)
                Data.append(d)
            if d.has_key('Include'):
                Include.append(d['Include'])
            if d.has_key('Extends'):
                Extends.append(d['Extends'])
    return {"Data":Data,"Include":Include,"Extends":Extends}

def ContainsData(type,name,list):
    for d in list:
        if d['Type']==type and d['Name']==name:
            return d['Class']
    return False

def ContainsInclude(name,list):
    for d in list:
        if d==name:
            return True
    return False

def ContainsExtends(name,list):
    for d in list:
        if d==name:
            return True
    return False

def CollectExtensions(Input,SearchDirs,Content,ClassName):
    content = ParseFile(Input)
    if content.has_key('Extends'):
        for e in content['Extends']:
            if not ContainsExtends(e,Content['Extends']):
                file=None
                ext=e.split('/')
                for d in SearchDirs:
                    ff = d+'/share/'+ext[0]+'/init/'+ext[1]+'.in'
                    if os.path.isfile(ff):
                        file = ff
                        break
                if not file:
                    eprint("Cannot find extension '"+e+"'!")
                Content['Extends'].append(e)
                ChildClassName = os.path.basename(file[0:-3])
                Content = CollectExtensions(file,SearchDirs,Content,ChildClassName)
    if content.has_key('Data'):
      for d in content['Data']:
          cls = ContainsData(d['Type'],d['Name'],Content['Data'])
          if cls:
              eprint("Property '"+d['Type']+" "+d['Name']+" in "+Input+" hides the parent's ("+cls+") property with the same id.")
              sys.exit(2)
          else:
              d['Class']=ClassName
              Content['Data'].append(d)
    if content.has_key('Include'):
        for i in content['Include']:
            if not ContainsInclude(i,Content['Include']):
                Content['Include'].append(i)
    return Content

def SortData(Data):
    a=[]
    b=[]
    for d in Data:
        if d['Required']:
            a.append(d)
        else:
            b.append(d)
    return a+b
def Generate(Input, Output, Namespace, ClassName,SearchDirs,DevelDir):
    print("Generating "+Output)
    content = CollectExtensions(Input,SearchDirs,{'Data':[],'Include':[],'Extends':[]},ClassName)
    txt=Construct(Namespace,ClassName+"Initializer",SortData(content["Data"]),content["Include"])
    dir=os.path.dirname(Output)
    if not os.path.exists(dir):
        os.makedirs(dir)
    with open(Output,"w") as f:
        f.write(txt)


if __name__ == "__main__":
    if len(sys.argv)>4:
        offset=4
        n=(len(sys.argv)-offset)/2
        Namespace=sys.argv[1]
        SearchDirs=sys.argv[2].split(':')
        print(SearchDirs)
        DevelDir=sys.argv[3]
        if not os.path.exists(DevelDir+'/init'):
            os.makedirs(DevelDir+'/init')

        for i in range(0,n):
            Input = sys.argv[offset+i]
            ClassName = os.path.basename(sys.argv[offset+i][0:-3])
            with open(Input,"r") as fi:
                with open(DevelDir+'/init/'+ClassName+'.in',"w") as f:
                    f.write(fi.read())

        for i in range(0,n):
            Input = sys.argv[offset+i]
            Output = sys.argv[offset+n+i]
            ClassName = os.path.basename(sys.argv[offset+i][0:-3])
            Generate(Input,Output,Namespace,ClassName,SearchDirs,DevelDir)
    else:
      eprint("Initializer generation failure: invalid arguments!")
      sys.exit(1)
