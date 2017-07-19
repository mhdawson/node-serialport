#include "./serialport.h"

#ifdef WIN32
  #define strncasecmp strnicmp
  #include "./serialport_win.h"
#else
  #include "./poller.h"
#endif

Napi::Value getValueFromObject(Napi::Object options, std::string key) {
  Napi::String v8str = Napi::String::New(options.Env(), key);
  return (options).Get(v8str);
}

int getIntFromObject(Napi::Object options, std::string key) {
  return getValueFromObject(options, key).ToNumber().Int64Value();
}

bool getBoolFromObject(Napi::Object options, std::string key) {
  return getValueFromObject(options, key).ToBoolean().Value();
}

Napi::String getStringFromObj(Napi::Object options, std::string key) {
  return getValueFromObject(options, key).ToString();
}

double getDoubleFromObject(Napi::Object options, std::string key) {
  return getValueFromObject(options, key).ToNumber().DoubleValue();
}

Napi::Value Open(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // path
  if (!info[0].IsString()) {
    Napi::TypeError::New(info.Env(), "First argument must be a string").ThrowAsJavaScriptException();
    return env.Null();
  }
  Napi::String path(env, info[0].ToString());

  // options
  if (!info[1].IsObject()) {
    Napi::TypeError::New(env, "Second argument must be an object").ThrowAsJavaScriptException();
    return env.Null();
  }
  Napi::Object options = info[1].ToObject();

  // callback
  if (!info[2].IsFunction()) {
    Napi::TypeError::New(env, "Third argument must be a function").ThrowAsJavaScriptException();
    return env.Null();
  }

  OpenBaton* baton = new OpenBaton();
  memset(baton, 0, sizeof(OpenBaton));
  strcpy(baton->path, *path);
  baton->baudRate = getIntFromObject(options, "baudRate");
  baton->dataBits = getIntFromObject(options, "dataBits");
  baton->parity = ToParityEnum(getStringFromObj(options, "parity"));
  baton->stopBits = ToStopBitEnum(getDoubleFromObject(options, "stopBits"));
  baton->rtscts = getBoolFromObject(options, "rtscts");
  baton->xon = getBoolFromObject(options, "xon");
  baton->xoff = getBoolFromObject(options, "xoff");
  baton->xany = getBoolFromObject(options, "xany");
  baton->hupcl = getBoolFromObject(options, "hupcl");
  baton->lock = getBoolFromObject(options, "lock");
  baton->callback.Reset(info[2].As<Napi::Function>());

  #ifndef WIN32
    baton->vmin = getIntFromObject(options, "vmin");
    baton->vtime = getIntFromObject(options, "vtime");
  #endif

  uv_work_t* req = new uv_work_t();
  req->data = baton;

  uv_queue_work(uv_default_loop(), req, EIO_Open, (uv_after_work_cb)EIO_AfterOpen);
}

void EIO_AfterOpen(uv_work_t* req) {
  Napi::HandleScope scope(env);

  OpenBaton* data = static_cast<OpenBaton*>(req->data);

  Napi::Value argv[2];
  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
    argv[1] = env.Undefined();
  } else {
    argv[0] = env.Null();
    argv[1] = Napi::Int32::New(env, data->result);
  }

  data->callback.Call(2, argv);
  delete data;
  delete req;
}

Napi::Value Update(const Napi::CallbackInfo& info) {
  // file descriptor
  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  int fd = info[0].As<Napi::Number>().Int32Value();

  // options
  if (!info[1].IsObject()) {
    Napi::TypeError::New(env, "Second argument must be an object").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  Napi::Object options = info[1].ToObject();

  if (!(options).Has(Napi::String::New(env, "baudRate")).FromMaybe(false)) {
    Napi::TypeError::New(env, "\"baudRate\" must be set on options object").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  // callback
  if (!info[2].IsFunction()) {
    Napi::TypeError::New(env, "Third argument must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  ConnectionOptionsBaton* baton = new ConnectionOptionsBaton();
  memset(baton, 0, sizeof(ConnectionOptionsBaton));

  baton->fd = fd;
  baton->baudRate = getIntFromObject(options, "baudRate");
  baton->callback.Reset(info[2].As<Napi::Function>());

  uv_work_t* req = new uv_work_t();
  req->data = baton;

  uv_queue_work(uv_default_loop(), req, EIO_Update, (uv_after_work_cb)EIO_AfterUpdate);
}

void EIO_AfterUpdate(uv_work_t* req) {
  Napi::HandleScope scope(env);

  ConnectionOptionsBaton* data = static_cast<ConnectionOptionsBaton*>(req->data);

  Napi::Value argv[1];
  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
  } else {
    argv[0] = env.Null();
  }

  data->callback.Call(1, argv);

  delete data;
  delete req;
}

Napi::Value Close(const Napi::CallbackInfo& info) {
  // file descriptor
  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  // callback
  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Second argument must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  VoidBaton* baton = new VoidBaton();
  memset(baton, 0, sizeof(VoidBaton));
  baton->fd = Napi::To<v8::Int32>(info[0])->Value();
  baton->callback.Reset(info[1].As<Napi::Function>());

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Close, (uv_after_work_cb)EIO_AfterClose);
}

void EIO_AfterClose(uv_work_t* req) {
  Napi::HandleScope scope(env);
  VoidBaton* data = static_cast<VoidBaton*>(req->data);

  Napi::Value argv[1];
  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
  } else {
    argv[0] = env.Null();
  }
  data->callback.Call(1, argv);

  delete data;
  delete req;
}

Napi::Value List(const Napi::CallbackInfo& info) {
  // callback
  if (!info[0].IsFunction()) {
    Napi::TypeError::New(env, "First argument must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  ListBaton* baton = new ListBaton();
  strcpy(baton->errorString, "");
  baton->callback.Reset(info[0].As<Napi::Function>());

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_List, (uv_after_work_cb)EIO_AfterList);
}

void setIfNotEmpty(Napi::Object item, std::string key, const char *value) {
  Napi::String v8key = Napi::String::New(env, key);
  if (strlen(value) > 0) {
    (item).Set(v8key, Napi::String::New(env, value));
  } else {
    (item).Set(v8key, env.Undefined());
  }
}

void EIO_AfterList(uv_work_t* req) {
  Napi::HandleScope scope(env);

  ListBaton* data = static_cast<ListBaton*>(req->data);

  Napi::Value argv[2];
  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
    argv[1] = env.Undefined();
  } else {
    Napi::Array results = Napi::Array::New(env);
    int i = 0;
    for (std::list<ListResultItem*>::iterator it = data->results.begin(); it != data->results.end(); ++it, i++) {
      Napi::Object item = Napi::Object::New(env);

      setIfNotEmpty(item, "comName", (*it)->comName.c_str());
      setIfNotEmpty(item, "manufacturer", (*it)->manufacturer.c_str());
      setIfNotEmpty(item, "serialNumber", (*it)->serialNumber.c_str());
      setIfNotEmpty(item, "pnpId", (*it)->pnpId.c_str());
      setIfNotEmpty(item, "locationId", (*it)->locationId.c_str());
      setIfNotEmpty(item, "vendorId", (*it)->vendorId.c_str());
      setIfNotEmpty(item, "productId", (*it)->productId.c_str());

      (results).Set(i, item);
    }
    argv[0] = env.Null();
    argv[1] = results;
  }
  data->callback.Call(2, argv);

  for (std::list<ListResultItem*>::iterator it = data->results.begin(); it != data->results.end(); ++it) {
    delete *it;
  }
  delete data;
  delete req;
}

Napi::Value Flush(const Napi::CallbackInfo& info) {
  // file descriptor
  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  int fd = info[0].As<Napi::Number>().Int32Value();

  // callback
  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Second argument must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  Napi::Function callback = info[1].As<Napi::Function>();

  VoidBaton* baton = new VoidBaton();
  memset(baton, 0, sizeof(VoidBaton));
  baton->fd = fd;
  baton->callback.Reset(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Flush, (uv_after_work_cb)EIO_AfterFlush);
}

void EIO_AfterFlush(uv_work_t* req) {
  Napi::HandleScope scope(env);

  VoidBaton* data = static_cast<VoidBaton*>(req->data);

  Napi::Value argv[1];

  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
  } else {
    argv[0] = env.Null();
  }

  data->callback.Call(1, argv);

  delete data;
  delete req;
}

Napi::Value Set(const Napi::CallbackInfo& info) {
  // file descriptor
  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  int fd = info[0].As<Napi::Number>().Int32Value();

  // options
  if (!info[1].IsObject()) {
    Napi::TypeError::New(env, "Second argument must be an object").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  Napi::Object options = info[1].ToObject();

  // callback
  if (!info[2].IsFunction()) {
    Napi::TypeError::New(env, "Third argument must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  Napi::Function callback = info[2].As<Napi::Function>();

  SetBaton* baton = new SetBaton();
  memset(baton, 0, sizeof(SetBaton));
  baton->fd = fd;
  baton->callback.Reset(callback);
  baton->brk = getBoolFromObject(options, "brk");
  baton->rts = getBoolFromObject(options, "rts");
  baton->cts = getBoolFromObject(options, "cts");
  baton->dtr = getBoolFromObject(options, "dtr");
  baton->dsr = getBoolFromObject(options, "dsr");

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Set, (uv_after_work_cb)EIO_AfterSet);
}

void EIO_AfterSet(uv_work_t* req) {
  Napi::HandleScope scope(env);

  SetBaton* data = static_cast<SetBaton*>(req->data);

  Napi::Value argv[1];

  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
  } else {
    argv[0] = env.Null();
  }
  data->callback.Call(1, argv);

  delete data;
  delete req;
}

Napi::Value Get(const Napi::CallbackInfo& info) {
  // file descriptor
  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  int fd = info[0].As<Napi::Number>().Int32Value();

  // callback
  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Second argument must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  GetBaton* baton = new GetBaton();
  memset(baton, 0, sizeof(GetBaton));
  baton->fd = fd;
  baton->cts = false;
  baton->dsr = false;
  baton->dcd = false;
  baton->callback.Reset(info[1].As<Napi::Function>());

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Get, (uv_after_work_cb)EIO_AfterGet);
}

void EIO_AfterGet(uv_work_t* req) {
  Napi::HandleScope scope(env);

  GetBaton* data = static_cast<GetBaton*>(req->data);

  Napi::Value argv[2];

  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
    argv[1] = env.Undefined();
  } else {
    Napi::Object results = Napi::Object::New(env);
    results->Set(Napi::String::New(env, "cts"), Napi::Boolean::New(env, data->cts));
    results->Set(Napi::String::New(env, "dsr"), Napi::Boolean::New(env, data->dsr));
    results->Set(Napi::String::New(env, "dcd"), Napi::Boolean::New(env, data->dcd));

    argv[0] = env.Null();
    argv[1] = results;
  }
  data->callback.Call(2, argv);

  delete data;
  delete req;
}

Napi::Value Drain(const Napi::CallbackInfo& info) {
  // file descriptor
  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "First argument must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  int fd = info[0].As<Napi::Number>().Int32Value();

  // callback
  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Second argument must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  VoidBaton* baton = new VoidBaton();
  memset(baton, 0, sizeof(VoidBaton));
  baton->fd = fd;
  baton->callback.Reset(info[1].As<Napi::Function>());

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Drain, (uv_after_work_cb)EIO_AfterDrain);
}

void EIO_AfterDrain(uv_work_t* req) {
  Napi::HandleScope scope(env);

  VoidBaton* data = static_cast<VoidBaton*>(req->data);

  Napi::Value argv[1];

  if (data->errorString[0]) {
    argv[0] = v8::Exception::Error(Napi::String::New(env, data->errorString));
  } else {
    argv[0] = env.Null();
  }
  data->callback.Call(1, argv);

  delete data;
  delete req;
}

SerialPortParity NAN_INLINE(ToParityEnum(const Napi::String& v8str)) {
  Napi::HandleScope scope(env);
  std::string str = v8str.As<Napi::String>();
  size_t count = strlen(*str);
  SerialPortParity parity = SERIALPORT_PARITY_NONE;
  if (!strncasecmp(*str, "none", count)) {
    parity = SERIALPORT_PARITY_NONE;
  } else if (!strncasecmp(*str, "even", count)) {
    parity = SERIALPORT_PARITY_EVEN;
  } else if (!strncasecmp(*str, "mark", count)) {
    parity = SERIALPORT_PARITY_MARK;
  } else if (!strncasecmp(*str, "odd", count)) {
    parity = SERIALPORT_PARITY_ODD;
  } else if (!strncasecmp(*str, "space", count)) {
    parity = SERIALPORT_PARITY_SPACE;
  }
  return parity;
}

SerialPortStopBits NAN_INLINE(ToStopBitEnum(double stopBits)) {
  if (stopBits > 1.4 && stopBits < 1.6) {
    return SERIALPORT_STOPBITS_ONE_FIVE;
  }
  if (stopBits == 2) {
    return SERIALPORT_STOPBITS_TWO;
  }
  return SERIALPORT_STOPBITS_ONE;
}

extern "C" {
  void init(v8::Handle<v8::Object> target) {
    Napi::HandleScope scope(env);
    Napi::SetMethod(target, "set", Set);
    Napi::SetMethod(target, "get", Get);
    Napi::SetMethod(target, "open", Open);
    Napi::SetMethod(target, "update", Update);
    Napi::SetMethod(target, "close", Close);
    Napi::SetMethod(target, "list", List);
    Napi::SetMethod(target, "flush", Flush);
    Napi::SetMethod(target, "drain", Drain);
    #ifdef WIN32
    Napi::SetMethod(target, "write", Write);
    Napi::SetMethod(target, "read", Read);
    #else
    Poller::Init(env, target, module);
    #endif
  }
}

NODE_API_MODULE(serialport, init);
