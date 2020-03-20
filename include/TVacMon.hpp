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

constexpr char ENQ = '\x05';
constexpr char ACK = '\x06';
constexpr char NAK = '\x15';

struct MonResult {
 public:
  MonResult(time_t t, double p, std::string name = "default")
  {
    TimeStamp = t;
    Pressure = p;
    SensorName = name;
  };

  time_t TimeStamp;
  double Pressure;
  std::string SensorName;
};

class TVacMon
{
 public:
  TVacMon();
  ~TVacMon();

  void InitPort();
  void SendCommand();
  void Read();

  void SetPortName(std::string name) { fPortName = name; };

  void SetTimeInterval(int v) { fTimeInterval = v; };

  void Terminate() { fAcqFlag = false; };

 private:
  std::unique_ptr<SerialPort> fPort;
  std::string fPortName;

  bool fAcqFlag;
  bool fReadWaitFlag;
  std::string fSensorName;

  bool CheckTime();
  int fTimeInterval;
  int fLastCheckTime;

  std::vector<MonResult> fBuffer;

  void Write(std::string com = "PA1");

  void DataWrite();

  void DataUpload();
  mongocxx::pool fPool;
};

#endif
