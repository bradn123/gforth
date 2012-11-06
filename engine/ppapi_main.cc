#include <ppapi/cpp/module.h>
#include <ppapi/cpp/instance.h>
#include <stdio.h>


class GforthInstance : public pp::Instance {
 public:
  explicit GforthInstance(PP_Instance instance) : pp::Instance(instance) {
  }

  virtual ~GforthInstance() {
  }

  void Run() {
    fprintf(stderr, "Run!\n");
  }

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    fprintf(stderr, "argc: %d\n", (int)argc);
    for (int i = 0; i < (int)argc; i++) {
      fprintf(stderr, "argn[%d]: %s\n", i, argn[i]);
      fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    }
    return true;
  }

  virtual void HandleMessage(const pp::Var& message) {
  }

 private:
  int xx;
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
