#ifndef SRC_SERIALPORT_WIN_H_
#define SRC_SERIALPORT_WIN_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <napi.h>
#include <uv.h>
#include <list>
#include <string>

#define ERROR_STRING_SIZE 1024

struct WriteBaton {
  int fd;
  char* bufferData;
  size_t bufferLength;
  size_t offset;
  size_t bytesWritten;
  Napi::Persistent<v8::Object> buffer;
  Napi::FunctionReference callback;
  int result;
  char errorString[ERROR_STRING_SIZE];
};

Napi::Value Write(const Napi::CallbackInfo& info);
void EIO_Write(uv_work_t* req);
void EIO_AfterWrite(uv_work_t* req);

struct ReadBaton {
  int fd;
  char* bufferData;
  size_t bufferLength;
  size_t bytesRead;
  size_t bytesToRead;
  size_t offset;
  char errorString[ERROR_STRING_SIZE];
  Napi::FunctionReference callback;
};

Napi::Value Read(const Napi::CallbackInfo& info);
void EIO_Read(uv_work_t* req);
void EIO_AfterRead(uv_work_t* req);
#endif  // SRC_SERIALPORT_WIN_H_
