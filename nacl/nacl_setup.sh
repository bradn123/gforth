# To be sourced.

export NACL_SDK_ROOT=~/nacl_sdk/pepper_21
export NACL_SDK_BIN=${NACL_SDK_ROOT}/toolchain/linux_x86_glibc/bin
export NACL_TOOLS=${NACL_SDK_ROOT}/tools
export NACL_SDK_LIB_BASE=${NACL_SDK_ROOT}/toolchain/linux_x86_glibc/x86_64-nacl/lib
export SRC_DIR="$(pwd)"
export TARGET_RUN="${SRC_DIR}/nacl/nacl_run "

function forbits() {
  export BITS=$1

  if [[ "$BITS" == "32" ]]; then
    export ARCH=i686
  else
    export ARCH=x86_64
  fi

  export NACL_SDK_LIB=${NACL_SDK_LIB_BASE}${BITS}
  export NACL_SEL_LDR=${NACL_TOOLS}/sel_ldr_x86_${BITS}
  export NACL_IRT=${NACL_TOOLS}/irt_x86_${BITS}.nexe
}

function config() {
  mkdir -p out/obj${BITS}
  cd out/obj${BITS}
  ../conf/configure \
      --srcdir=${SRC_DIR} \
      --host=${ARCH}-nacl \
      CC="${NACL_SDK_BIN}/${ARCH}-nacl-gcc -DNACL_BITS=${BITS}"
  cd ${SRC_DIR}
}

function  genconfig() {
  mkdir out/conf
  mkdir out/conf/engine
  cp *.in out/conf
  cp autogen.sh out/conf
  cd out/conf
  ./autogen.sh
  cd ../..
}
