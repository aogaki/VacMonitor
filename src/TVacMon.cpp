#include <unistd.h>

#include <ctime>
#include <iostream>

#include "TVacMon.hpp"

TVacMon::TVacMon()
    : fPortName("/dev/ttyUSB0"), fAcqFlag(true), fTimeInterval(60)
{
  fLastCheckTime = time(nullptr);
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
        }
        buf.clear();
        readFlag = false;
      }
    }
  }
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

void TVacMon::PlotGraph() {}

void TVacMon::DataWrite() {}
