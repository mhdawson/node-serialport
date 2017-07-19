#include <napi.h>
#include <uv.h>
#include "./poller.h"

Poller::Poller(int fd) {
  Napi::HandleScope scope(env);
  this->fd = fd;
  this->poll_handle = new uv_poll_t();
  memset(this->poll_handle, 0, sizeof(uv_poll_t));
  poll_handle->data = this;
  int status = uv_poll_init(uv_default_loop(), poll_handle, fd);
  if (0 != status) {
    Napi::Error::New(env, uv_strerror(status)).ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  uv_poll_init_success = true;
}

Poller::~Poller() {
  // if we call uv_poll_stop after uv_poll_init failed we segfault
  if (uv_poll_init_success) {
    uv_poll_stop(poll_handle);
    uv_close((uv_handle_t*) poll_handle, Poller::onClose);
  } else {
    delete poll_handle;
  }
}

void Poller::onClose(uv_handle_t* poll_handle) {
  // fprintf(stdout, "~Poller is closed\n");
  delete poll_handle;
}

// Events can be UV_READABLE | UV_WRITABLE | UV_DISCONNECT
void Poller::poll(int events) {
  // fprintf(stdout, "Poller:poll for %d\n", events);
  this->events = this->events | events;
  int status = uv_poll_start(poll_handle, events, Poller::onData);
  if (0 != status) {
    Napi::TypeError::New(env, uv_strerror(status)).ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
}

void Poller::stop() {
  int status = uv_poll_stop(poll_handle);
  if (0 != status) {
    Napi::TypeError::New(env, uv_strerror(status)).ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
}

void Poller::onData(uv_poll_t* handle, int status, int events) {
  Napi::HandleScope scope(env);
  Poller* obj = static_cast<Poller*>(handle->data);
  Napi::Value argv[2];
  if (0 != status) {
    // fprintf(stdout, "OnData Error status=%s events=%d\n", uv_strerror(status), events);
    argv[0] = v8::Exception::Error(Napi::String::New(env, uv_strerror(status)));
    argv[1] = env.Undefined();
  } else {
    // fprintf(stdout, "OnData status=%d events=%d\n", status, events);
    argv[0] = env.Null();
    argv[1] = Napi::Number::New(env, events);
  }
  // remove triggered events from the poll
  int newEvents = obj->events & ~events;
  obj->poll(newEvents);

  obj->callback.Call(2, argv);
}

void Poller::Init(Napi::Env env, Napi::Object exports, Napi::Object module) {
  Napi::FunctionReference tpl = Napi::Function::New(env, New);
  tpl->SetClassName(Napi::String::New(env, "Poller"));


    InstanceMethod("poll", &poll),
    InstanceMethod("stop", &stop),

  constructor().Reset(Napi::GetFunction(tpl));
  (target).Set(Napi::String::New(env, "Poller"), Napi::GetFunction(tpl));
}

Napi::Value Poller::New(const Napi::CallbackInfo& info) {
  if (!info.IsConstructCall()) {
    const int argc = 2;
    Napi::Value argv[argc] = {info[0], info[1]};
    Napi::Function cons = Napi::New(env, constructor());
    return Napi::NewInstance(cons, argc, argv);
    return;
  }

  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "fd must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  int fd = info[0].As<Napi::Number>().Int32Value();

  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "cb must be a function").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }

  Poller *obj = new Poller(fd);
  obj->callback.Reset(info[1].As<Napi::Function>());
  obj->Wrap(info.This());
  return info.This();
}

Napi::Value Poller::poll(const Napi::CallbackInfo& info) {
  Poller* obj = info.Holder().Unwrap<Poller>();
  if (!info[0].IsNumber()) {
    Napi::TypeError::New(env, "events must be an int").ThrowAsJavaScriptException();
  return env.Null();
    return;
  }
  int events = info[0].As<Napi::Number>().Int32Value();
  obj->poll(events);
}

Napi::Value Poller::stop(const Napi::CallbackInfo& info) {
  Poller* obj = info.Holder().Unwrap<Poller>();
  obj->stop();
}

inline Napi::FunctionReference & Poller::constructor() {
  static Napi::FunctionReference my_constructor;
  return my_constructor;
}
