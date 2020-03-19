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
}

void TVacMon::Write(std::string com)
{
  fPort->Write(com);
<<<<<<< HEAD
  fPort->WriteByte(ENQ);
=======
  Read();
  fPort->WriteByte(ENQ);
  Read();
>>>>>>> 7e8d3a3f440ae62148f210ceaf137e448ec0ffb4

  while (fAcqFlag) {
    if (CheckTime()) {
      fPort->Write(com);
<<<<<<< HEAD
      fPort->WriteByte(ENQ);
=======
      Read();
      fPort->WriteByte(ENQ);
      Read();
>>>>>>> 7e8d3a3f440ae62148f210ceaf137e448ec0ffb4
    }

    usleep(10);
  }
}

void TVacMon::Read()
{
  bool readFlag = true;
  std::string buf{""};
  const unsigned int timeout_ms = 25;  // timeout value in milliseconds

<<<<<<< HEAD
  while (fAcqFlag) {
    try {
      auto c = fPort->ReadByte(timeout_ms);
      readFlag = true;
      buf += c;
    } catch (const SerialPort::ReadTimeout &timeOut) {
      if (readFlag) {
        if (buf != "") {
          if (buf.find_first_of(',') != std::string::npos)
            std::cout << buf << std::endl;
          auto start = buf.find_first_of(',') + 1;  // next of ","
          auto pressure = std::stod(buf.substr(start, buf.size() - start));
          auto timeStamp = time(nullptr);

          fBuffer.push_back(MonResult(timeStamp, pressure));
        }
        buf.clear();
        readFlag = false;

        // if (fBuffer.size() > 10) DataWrite();
        DataWrite();
=======
  try {
    auto c = fPort->ReadByte(timeout_ms);
    readFlag = true;
    buf += c;
  } catch (const SerialPort::ReadTimeout &timeOut) {
    if (readFlag) {
      if (buf != "") {
        if (buf.find_first_of(',') != std::string::npos)
          std::cout << buf << std::endl;
        auto start = buf.find_first_of(',') + 1;  // next of ","
        auto pressure = std::stod(buf.substr(start, buf.size() - start));
        auto timeStamp = time(nullptr);

        fBuffer.push_back(MonResult(timeStamp, pressure));
>>>>>>> 7e8d3a3f440ae62148f210ceaf137e448ec0ffb4
      }
      buf.clear();
      readFlag = false;

      // if (fBuffer.size() > 10) DataWrite();
      DataWrite();
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
