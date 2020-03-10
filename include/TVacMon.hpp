#ifndef TVacMon_hpp
#define TVacMon_hpp 1

#include <SerialPort.h>
#include <SerialStream.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <THttpServer.h>

#include <memory>
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

  void PlotGraph();
  std::unique_ptr<THttpServer> fServer;
  std::unique_ptr<TGraph> fGraph;
  std::unique_ptr<TCanvas> fCanvas;
  std::vector<MonResult> fData;
  std::vector<MonResult> fBuffer;

  // Data output is needed.  Ask Anukul what he want
  void DataWrite();
  std::mutex fDataWriteMutex;
};

#endif
