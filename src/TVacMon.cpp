#include <TAxis.h>
#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iostream>

#include "TVacMon.hpp"

TVacMon::TVacMon()
    : fPortName("/dev/ttyUSB0"),
      fAcqFlag(true),
      fTimeInterval(60),
      fPool(mongocxx::uri("mongodb://daq:nim2camac@172.18.4.56/ELIADE"))
{
  fLastCheckTime = time(nullptr);

  fGraph.reset(new TGraph());
  fGraph->SetTitle("Pressure monitor;Time;Pressure [mbar]");
  fGraph->SetMaximum(5.e+2);
  fGraph->SetMinimum(5.e-8);
  fGraph->SetMarkerStyle(8);
  fGraph->SetMarkerColor(kRed);
  fGraph->GetXaxis()->SetTimeDisplay(1);
  fGraph->GetXaxis()->SetTimeFormat("%H:%M");

  fCanvas.reset(new TCanvas("canvas", "Pressure monitor"));
  fCanvas->SetLogy(kTRUE);

  fServer.reset(new THttpServer());
}

TVacMon::~TVacMon() { fPort->Close(); }

void TVacMon::InitPort()
{
  fPort.reset(new SerialPort(fPortName));
  // fPort = SerialPort(fPortName);

  fPort->Open();
  fPort->SetBaudRate(SerialPort::BaudRate::BAUD_9600);
  fPort->SetCharSize(SerialPort::CharacterSize::CHAR_SIZE_8);
  fPort->SetFlowControl(SerialPort::FlowControl::FLOW_CONTROL_NONE);
  fPort->SetParity(SerialPort::Parity::PARITY_NONE);
  fPort->SetNumOfStopBits(SerialPort::StopBits::STOP_BITS_1);

  std::string com = "PA1";
  fPort->Write(com);
}

void TVacMon::Write()
{
  fPort->WriteByte('\x05');

  while (fAcqFlag) {
    if (CheckTime()) {
      fPort->WriteByte('\x05');
    }

    usleep(10);
  }
}

void TVacMon::Read()
{
  bool readFlag = true;
  std::string buf{""};
  const unsigned int timeout_ms = 25;  // timeout value in milliseconds

  while (fAcqFlag) {
    try {
      auto c = fPort->ReadByte(timeout_ms);
      readFlag = true;
      buf += c;
    } catch (const SerialPort::ReadTimeout &timeOut) {
      if (readFlag) {
        if (buf != "") {
          auto start = buf.find_first_of(',') + 1;  // next of ","
          auto pressure = std::stod(buf.substr(start, buf.size() - start));
          auto timeStamp = time(nullptr);
          fData.push_back(MonResult(timeStamp, pressure));

          PlotGraph();

          std::lock_guard<std::mutex> lock(fDataWriteMutex);
          fBuffer.push_back(MonResult(timeStamp, pressure));
        }
        buf.clear();
        readFlag = false;

        if (fBuffer.size() > 10) DataWrite();
      }
    }
  }

  DataWrite();
}

bool TVacMon::CheckTime()
{
  auto now = time(nullptr);
  if (now - fLastCheckTime > fTimeInterval) {
    fLastCheckTime = now;
    return true;
  } else {
    return false;
  }
}

void TVacMon::PlotGraph()
{
  unsigned int start = 0;

  constexpr unsigned int plotLimit = 100;
  const unsigned int dataSize = fData.size();
  if (dataSize > plotLimit) start = dataSize - plotLimit;

  for (unsigned int i = start; i < dataSize; i++) {
    fGraph->SetPoint(i, fData[i].TimeStamp, fData[i].Pressure);
  }

  std::lock_guard<std::mutex> lock(fPlotGraphMutex);
  fCanvas->cd();
  fGraph->Draw("ALP");
  fCanvas->Modified();
  fCanvas->Update();
}

void TVacMon::DataWrite()
{
  if (!fBuffer.empty()) {
    // Also upload the data
    DataUpload();

    auto fileName = "monitor.log";
    std::fstream fout(fileName, std::ios::app);

    for (unsigned int i = 0; i < fBuffer.size(); i++) {
      fout << fBuffer[i].TimeStamp << "\t" << fBuffer[i].Pressure << std::endl;
    }

    fout.close();
    std::lock_guard<std::mutex> lock(fDataWriteMutex);
    fBuffer.clear();
  }
}

void TVacMon::DataUpload()
{
  auto conn = fPool.acquire();
  auto collection = (*conn)["ELIADE"]["VacMon"];

  bsoncxx::builder::stream::document buf{};

  for (unsigned int i = 0; i < fBuffer.size(); i++) {
    buf << "time" << fBuffer[i].TimeStamp << "pressure" << fBuffer[i].Pressure;
    collection.insert_one(buf.view());
    buf.clear();
  }
}
