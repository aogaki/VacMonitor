#ifndef TVacMon_hpp
#define TVacMon_hpp 1

#include <SerialPort.h>
#include <SerialStream.h>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <memory>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mutex>
#include <string>
#include <vector>

using namespace LibSerial;

struct MonResult {
 public:
  MonResult(time_t t, double p)
  {
    TimeStamp = t;
    Pressure = p;
  };

  time_t TimeStamp;
  double Pressure;
};

class TVacMon
{
 public:
  TVacMon();
  ~TVacMon();

  void InitPort();
  void Write();
  void Read();

  void SetPortName(std::string name) { fPortName = name; };

  void SetTimeInterval(int v) { fTimeInterval = v; };

  void Terminate() { fAcqFlag = false; };

 private:
  std::unique_ptr<SerialPort> fPort;
  std::string fPortName;

  bool fAcqFlag;

  bool CheckTime();
  int fTimeInterval;
  int fLastCheckTime;

  std::vector<MonResult> fBuffer;

  void DataWrite();

  void DataUpload();
  mongocxx::pool fPool;
};

#endif
