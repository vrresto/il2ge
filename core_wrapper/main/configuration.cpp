#include "configuration_p.h"
#include <log.h>

#include <INIReader.h>
#include <windows.h>


extern "C" WINAPI void il2geUpdateConfig();

using namespace std;


namespace
{
  constexpr auto CONFIG_FILE_NAME = "il2ge.ini";

  il2ge::core_wrapper::Configuration g_config;
}


namespace il2ge::core_wrapper
{


const Configuration &getConfig()
{
  return g_config;
}


void readConfig()
{
  INIReader ini(CONFIG_FILE_NAME);

  if (!ini.ParseError())
  {
    auto read_value = [&ini] (std::string name)
    {
      return ini.Get("", name, "");
    };

    g_config.read(read_value);
  }
  else if (ini.ParseError() != -1)
  {
    LOG_ERROR << "Error reading configuration file " << CONFIG_FILE_NAME
      << " at line " << ini.ParseError() << endl
      << "Using default configuration." << endl;
  }

  LOG_INFO << endl;
  LOG_INFO << "*** IL2GE Configuration ***" << endl;

  for (auto &setting : g_config.getSettings())
  {
    LOG_INFO << setting->getName() << ": " << setting->getValueStr() << endl;
  }

  LOG_INFO << endl;
  LOG_FLUSH;
}


void writeConfig()
{
  ofstream config_out(CONFIG_FILE_NAME);
  assert(config_out.good());
  g_config.write(config_out);
}


} // namespace il2ge::core_wrapper


extern "C" WINAPI void il2geUpdateConfig()
{
  il2ge::core_wrapper::readConfig();
  il2ge::core_wrapper::writeConfig();
}
