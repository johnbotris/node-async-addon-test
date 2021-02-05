#include <nan.h>

#include <iostream>

#include "SPSCQueue.h"

using std::cout;
using std::endl;
using std::shared_ptr;

using v8::FunctionTemplate;
using v8::String;

using v8::External;
using v8::Function;
using v8::Local;
using v8::Promise;
using v8::Value;

using Queue = rigtorp::SPSCQueue<std::string>;

class Worker : public Nan::AsyncWorker {
   private:
    std::shared_ptr<Queue> queue;

   public:
    Worker(Nan::Callback *callback, std::shared_ptr<Queue> queue)
        : Nan::AsyncWorker(callback, "DoAsyncStuff::PiWorker"), queue(queue) {}

    ~Worker() {}

    virtual void Execute() override {
        cout << "native is waiting for javascript..." << endl;
        std::string *data = nullptr;
        while ((data = this->queue->front()) == nullptr) {
        }
        std::cout << "got some data from javascript: \"" << *data << '"'
                  << std::endl;
        this->queue->pop();
    }

   protected:
    virtual void HandleOKCallback() override {
        Nan::HandleScope scope;

        const int argc = 2;
        Local<Value> args[argc]{
            Nan::Null(),
            Nan::New<String>("native stuff finished, bye").ToLocalChecked()};

        this->callback->Call(argc, args, this->async_resource);
    }
};

NAN_METHOD(DoAsyncStuff) {
    Local<Promise> dataPromise = info[0].As<Promise>();

    // New shared pointer goes on heap, copy of it goes to worker
    // when promise is resolved, the one on heap get's deleted but the one in
    // the worker remains alive. when the worker gets destructed, the queue goes
    // along with it
    // TODO find a c++ guru to confirm if this is true
    std::shared_ptr<Queue> *queuePtr = new std::shared_ptr<Queue>(new Queue(3));

    Nan::Callback *workerCallback =
        new Nan::Callback(Nan::To<Function>(info[1]).ToLocalChecked());

    Worker *worker =
        new Worker(workerCallback, std::shared_ptr<Queue>(*queuePtr));

    typedef Nan::NAN_METHOD_ARGS_TYPE ArgsType;
    typedef Nan::NAN_METHOD_RETURN_TYPE ReturnType;

    auto dataReadyHandler = [](ArgsType info) -> ReturnType {
        Local<External> data = info.Data().As<External>();
        shared_ptr<Queue> *queue =
            static_cast<shared_ptr<Queue> *>(data->Value());

        std::string arg = info[0]->IsString()
                              ? *Nan::Utf8String(info[0].As<String>())
                              : "<unknown data>";

        // TODO here is where we put whatever data we were waiting for
        (*queue)->push(arg);
        delete queue;
    };

    auto dataReadyCallback = Nan::GetFunction(Nan::New<FunctionTemplate>(
        dataReadyHandler, Nan::New<External>(static_cast<void *>(queuePtr))));

    dataPromise->Then(Nan::GetCurrentContext(),
                      dataReadyCallback.ToLocalChecked());

    Nan::AsyncQueueWorker(worker);
}

NAN_MODULE_INIT(Init) { NAN_EXPORT(target, DoAsyncStuff); }

NODE_MODULE(addon, Init)
