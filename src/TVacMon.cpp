#include <TAxis.h>
#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iostream>

#include "TVacMon.hpp"

TVacMon::TVacMon()
    : fPortName("/dev/ttyUSB0"), fAcqFlag(true), fTimeInterval(60)
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
          std::cout << timeStamp << "\t" << pressure << std::endl;
          fData.push_back(MonResult(timeStamp, pressure));

          PlotGraph();
        }
        buf.clear();
        readFlag = false;
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

  fCanvas->cd();
  fGraph->Draw("ALP");
  fCanvas->Modified();
  fCanvas->Update();
}

void TVacMon::DataWrite()
{
  auto fileName = "monitor.log";
  std::fstream fout(fileName, std::ios::app);

  for (unsigned int i = 0; i < fData.size(); i++) {
    fout << fData[i].TimeStamp << "\t" << fData[i].Pressure << "\n";
  }

  fout << std::endl;
  fout.close();
}
