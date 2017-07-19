#ifndef SRC_SERIALPORT_H_
#define SRC_SERIALPORT_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <napi.h>
#include <uv.h>
#include <list>
#include <string>

#define ERROR_STRING_SIZE 1024

Napi::Value List(const Napi::CallbackInfo& info);
void EIO_List(uv_work_t* req);
void EIO_AfterList(uv_work_t* req);

Napi::Value Open(const Napi::CallbackInfo& info);
void EIO_Open(uv_work_t* req);
void EIO_AfterOpen(uv_work_t* req);

Napi::Value Update(const Napi::CallbackInfo& info);
void EIO_Update(uv_work_t* req);
void EIO_AfterUpdate(uv_work_t* req);

Napi::Value Close(const Napi::CallbackInfo& info);
void EIO_Close(uv_work_t* req);
void EIO_AfterClose(uv_work_t* req);

Napi::Value Flush(const Napi::CallbackInfo& info);
void EIO_Flush(uv_work_t* req);
void EIO_AfterFlush(uv_work_t* req);

Napi::Value Set(const Napi::CallbackInfo& info);
void EIO_Set(uv_work_t* req);
void EIO_AfterSet(uv_work_t* req);

Napi::Value Get(const Napi::CallbackInfo& info);
void EIO_Get(uv_work_t* req);
void EIO_AfterGet(uv_work_t* req);

Napi::Value Drain(const Napi::CallbackInfo& info);
void EIO_Drain(uv_work_t* req);
void EIO_AfterDrain(uv_work_t* req);

enum SerialPortParity {
  SERIALPORT_PARITY_NONE  = 1,
  SERIALPORT_PARITY_MARK  = 2,
  SERIALPORT_PARITY_EVEN  = 3,
  SERIALPORT_PARITY_ODD   = 4,
  SERIALPORT_PARITY_SPACE = 5
};

enum SerialPortStopBits {
  SERIALPORT_STOPBITS_ONE      = 1,
  SERIALPORT_STOPBITS_ONE_FIVE = 2,
  SERIALPORT_STOPBITS_TWO      = 3
};

SerialPortParity ToParityEnum(const Napi::String& str);
SerialPortStopBits ToStopBitEnum(double stopBits);

struct OpenBaton {
  char errorString[ERROR_STRING_SIZE];
  Napi::FunctionReference callback;
  char path[1024];
  int fd;
  int result;
  int baudRate;
  int dataBits;
  bool rtscts;
  bool xon;
  bool xoff;
  bool xany;
  bool dsrdtr;
  bool hupcl;
  bool lock;
  SerialPortParity parity;
  SerialPortStopBits stopBits;
#ifndef WIN32
  uint8_t vmin;
  uint8_t vtime;
#endif
};

struct ConnectionOptionsBaton {
  char errorString[ERROR_STRING_SIZE];
  Napi::FunctionReference callback;
  int fd;
  int baudRate;
};

struct ListResultItem {
  std::string comName;
  std::string manufacturer;
  std::string serialNumber;
  std::string pnpId;
  std::string locationId;
  std::string vendorId;
  std::string productId;
};

struct ListBaton {
  Napi::FunctionReference callback;
  std::list<ListResultItem*> results;
  char errorString[ERROR_STRING_SIZE];
};

struct SetBaton {
  int fd;
  Napi::FunctionReference callback;
  int result;
  char errorString[ERROR_STRING_SIZE];
  bool rts;
  bool cts;
  bool dtr;
  bool dsr;
  bool brk;
};

struct GetBaton {
  int fd;
  Napi::FunctionReference callback;
  char errorString[ERROR_STRING_SIZE];
  bool cts;
  bool dsr;
  bool dcd;
};

struct VoidBaton {
  int fd;
  Napi::FunctionReference callback;
  char errorString[ERROR_STRING_SIZE];
};

int setup(int fd, OpenBaton *data);
int setBaudRate(ConnectionOptionsBaton *data);
#endif  // SRC_SERIALPORT_H_
