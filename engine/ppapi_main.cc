#include <ppapi/cpp/module.h>
#include <ppapi/cpp/instance.h>
#include <stdio.h>
#include "nacl-mounts/base/UrlLoaderJob.h"
#include "nacl-mounts/console/JSPipeMount.h"
#include "nacl-mounts/console/JSPostMessageBridge.h"
#include "nacl-mounts/pepper/PepperMount.h"


//#define USE_PSEUDO_THREADS


extern "C" int main(int argc, char *argv[]);
extern "C" int umount(const char *path);
extern "C" int mount(const char *source, const char *target,
    const char *filesystemtype, unsigned long mountflags, const void *data);
extern "C" int simple_tar_extract(const char *path);


static void Download(MainThreadRunner* runner,
                     const char *url, const char *filename) {
  UrlLoaderJob* job = new UrlLoaderJob;
  job->set_url(url);
  std::vector<char> data;
  job->set_dst(&data);
  runner->RunJob(job);
  int fh = open(filename, O_CREAT | O_WRONLY);
  write(fh, &data[0], data.size());
  close(fh);
}


static void *gforth_init(void *arg) {
  MainThreadRunner* runner = reinterpret_cast<MainThreadRunner*>(arg);

  chdir("/");
#if NACL_BITS == 32
  Download(runner, "gforth32.tar", "/gforth.tar");
#elif NACL_BITS == 64
  Download(runner, "gforth64.tar", "/gforth.tar");
#else
# error "Bad NACL_BITS value"
#endif
  simple_tar_extract("/gforth.tar");

  setenv("OUTSIDE_BROWSER", "1", 1);
  const char *argv[] = {"gforth"};
  main(1, const_cast<char**>(argv));
  return 0;
}


class GforthInstance : public pp::Instance {
 public:
  explicit GforthInstance(PP_Instance instance) : pp::Instance(instance) {
    jspipe_ = NULL;
    jsbridge_ = NULL;
  }

  virtual ~GforthInstance() {
    if (jspipe_) delete jspipe_;
    if (jsbridge_) delete jsbridge_;
    if (runner_) delete runner_;
  }

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    runner_ = new MainThreadRunner(this);
    jsbridge_ = new JSPostMessageBridge(runner_);
    jspipe_ = new JSPipeMount();
#ifdef USE_PSEUDO_THREADS
    jspipe_->set_using_pseudo_thread(true);
#endif
    jspipe_->set_outbound_bridge(jsbridge_);
    // Replace stdin, stdout, stderr with js console.
    mount(0, "/jspipe", 0, 0, jspipe_);
    close(0);
    close(1);
    close(2);
    int fd;
    fd = open("/jspipe/0", O_RDONLY);
    assert(fd == 0);
    fd = open("/jspipe/1", O_WRONLY);
    assert(fd == 1);
    fd = open("/jspipe/2", O_WRONLY);
    assert(fd == 2);

    // Mount local storage.
    {
      mkdir("/save", 0777);
      pp::FileSystem *fs = new pp::FileSystem(
          this, PP_FILESYSTEMTYPE_LOCALPERSISTENT);
      PepperMount* pm = new PepperMount(runner_, fs, 100 * 1024 * 1024);
      pm->SetPathPrefix("/save");
      mount(0, "/save", 0, 0, pm);
    }

#ifdef USE_PSEUDO_THREADS
    runner_->PseudoThreadFork(gforth_init, runner_);
#else
    pthread_create(&gforth_thread_, NULL, gforth_init, runner_);
#endif

    return true;
  }

  virtual void HandleMessage(const pp::Var& message) {
    std::string msg = message.AsString();
    jspipe_->Receive(msg.c_str(), msg.size());
  }

 private:
  pthread_t gforth_thread_;
  JSPipeMount* jspipe_;
  JSPostMessageBridge* jsbridge_;
  MainThreadRunner* runner_;
};


class GforthModule : public pp::Module {
 public:
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new GforthInstance(instance);
  }
  virtual ~GforthModule() { }
};


namespace pp {
pp::Module *CreateModule(void) {
  return new GforthModule();
}
};
