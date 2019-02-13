/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2018 Jan Lepper
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <util.h>

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <cassert>

using namespace std;

namespace
{


struct MethodInfo
{
  string name;
  string return_type;
  vector<string> arg_types;
};

struct ClassInfo
{
  vector<MethodInfo> methods;
};

map<string, ClassInfo> g_classes;

ClassInfo &getClassInfo(const string &name)
{
  return g_classes[name];
}

void parseSignatures(istream &in)
{
  while (in.good())
  {
    string line;
    getline(in, line);
    istringstream line_stream(line);

    MethodInfo mi;

    line_stream >> mi.return_type;
//     cerr<<"mi.return_type: "<<mi.return_type<<endl;
    if(mi.return_type.empty())
      continue;

    string class_name;
    line_stream >> class_name;
//     cerr<<"class name: "<<class_name<<endl;
    if (class_name.empty())
      continue;

    line_stream >> mi.name;
    if (mi.name.empty())
      continue;

    while (line_stream.good())
    {
      string arg_type;
      line_stream >> arg_type;
      mi.arg_types.push_back(arg_type);
    }

    ClassInfo &ci = getClassInfo(class_name);
    ci.methods.push_back(mi);
  }
}

void emitMethodDefinitions(const ClassInfo &info, ostream &out)
{
  for (const MethodInfo &m : info.methods)
  {
    out << "typedef MethodSpec<" << m.return_type;
    for (size_t i = 0; i < m.arg_types.size(); i++)
    {
      out << ", " << m.arg_types[i];
    }
    out << "> " << m.name << "_t;" << endl;
  }
}

void emitInterfaceDefinition(const ClassInfo &info, ostream &out)
{
  out << "struct Interface" << endl;
  out << "{" << endl;
  for (const MethodInfo &m : info.methods)
  {
    out << '\t' << m.name << "_t::Signature *" << m.name << " = nullptr;" << endl;
  }
  out << "};" << endl;
}

void emitMetaClassRegistration(const string &name, ostream &out)
{
  ClassInfo &info = getClassInfo(name);

  size_t pos = name.find_last_of('.');
  assert(pos != std::string::npos);
  assert(pos < name.size()-1);

  string package = name.substr(0, pos);
  string class_name = name.substr(pos + 1);

  auto path_tokens = util::tokenize(package, '.');
  assert(!path_tokens.empty());

  out << "void ::jni_wrapper::registrator::";

  for (size_t i = 0; i < path_tokens.size(); i++)
  {
    out << path_tokens[i] << "::";
  }

  out << class_name << "(MetaClass &meta_class)" << endl;

  out << "{" << endl;
  out << '\t' << "meta_class.package = \"com.maddox." << package << "\";" <<endl;
  out << '\t' << "meta_class.name = \"" << class_name << "\";" << endl;

  for (MethodInfo &mi : info.methods)
  {
    out << '\t' << "meta_class.addMethod<" << mi.name <<
      "_t>(\"" << mi.name << "\", &::import." << mi.name << ", &::" << mi.name << ");" << endl;
  }

  out << "}" << endl;
  out << endl;
}

void emitMethodImplementation(const MethodInfo &mi, ostream &out)
{
  // head
  out << mi.return_type << " JNICALL " << mi.name << "(JNIEnv *env, jobject obj";
  for (size_t i = 0; i < mi.arg_types.size(); i++)
  {
    out << "," << endl << "\t\t" << mi.arg_types[i] << " arg" << i;
  }
  out << ")" << endl;

  // body
  out << "{" << endl;
  out << "\t" << "return import." << mi.name << "(env, obj";
  for (size_t i = 0; i < mi.arg_types.size(); i++)
  {
    out << "," << " arg" << i;
  }
  out << ");" << endl;
  out << "}" << endl;

  out << endl;
}

void dumpClassWrappers(const string &output_dir)
{
  for (auto it : g_classes)
  {
    string class_name = it.first;
    const ClassInfo &ci = it.second;

    ofstream out(output_dir + '/' + "wrap_" + class_name + ".cpp");
    assert(out.good());

    out << "#include \"jni_wrapper.h\"" << endl;
    out << "#include \"meta_class_registrators.h\"" << endl;
    out << endl;
    out << "using namespace jni_wrapper;" << endl;
    out << endl;
    out << "namespace" << endl;
    out << "{" << endl;
    out << endl;
    out << "#include <_generated/jni_wrapper/" << class_name << "_definitions>" << endl;
    out << endl;
    out << "Interface import;" << endl;
    out << endl;

    for (const MethodInfo &mi : ci.methods)
    {
      emitMethodImplementation(mi, out);
    }

    out << endl;
    out << "} // namespace" << endl;

    out << endl;
    out << endl;
    out << "#include <_generated/jni_wrapper/" << class_name << "_registration>" << endl;
  }
}


} // namespace


int main(int argc, char **argv)
{
  assert(argc == 3);

  string cmd = argv[1];
  assert(!cmd.empty());

  string cmd_arg = argv[2];
  assert(!cmd_arg.empty());

  if(cmd == "registrator-table")
  {
    for (auto name : util::tokenize(cmd_arg))
    {
      auto path_tokens = util::tokenize(name, '.');
      assert(!path_tokens.empty());

      cout << "&::jni_wrapper::registrator::";

      for (size_t i = 0; i < path_tokens.size() - 1; i++)
      {
        cout << path_tokens[i] << "::";
      }

      cout << path_tokens.back() << "," << endl;
    }
  }
  else if(cmd == "registrator-definitions")
  {
    for (auto name : util::tokenize(cmd_arg))
    {
      auto path_tokens = util::tokenize(name, '.');
      assert(!path_tokens.empty());
      assert(path_tokens.size() > 1);

      cout << "namespace jni_wrapper::registrator";

      for (size_t i = 0; i < path_tokens.size() - 1; i++)
      {
        cout << "::" << path_tokens[i];
      }

      cout << " { ";
      cout << "MetaClassInitFunc " << path_tokens.back() << ";";
      cout << " }" << endl;
    }
  }
  else if (cmd == "definitions")
  {
    parseSignatures(cin);

    string class_name = cmd_arg;

    emitMethodDefinitions(getClassInfo(class_name), cout);
    cout << endl;
    emitInterfaceDefinition(getClassInfo(class_name), cout);
  }
  else if (cmd == "registration")
  {
    parseSignatures(cin);

    string class_name = cmd_arg;
//     class_name[0] = toupper(class_name[0]);

    emitMetaClassRegistration(class_name, cout);
  }
  else if (cmd == "all")
  {
    parseSignatures(cin);

    dumpClassWrappers(cmd_arg);
  }
  else
  {
    assert(0);
  }
}
