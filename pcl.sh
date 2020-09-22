#!/bin/bash

file_path=""
asm_out=false
ir_out=false
opt_flag=-O0
DIR=$(pwd)
pcl_compiler=$DIR/pcl
LIB_OBJ=$DIR/lib.o
TEMP=_pcl_temp
mtriple=$(clang -dumpmachine)
while [[ $# -gt 0 ]];
do
    case "$1" in
        -O0)  opt_flag=-O0;;
        -O1)  opt_flag=-O1;;
        -O2)  opt_flag=-O2;;
        -O3)  opt_flag=-O3;;
        -O)   opt_flag=-O2;;
        -i)   ir_out=true;;
        -f)   asm_out=true;;
        -*)   ;;
        *)    file_path="$1"
    esac
    shift
done



if [[ ${ir_out} = true ]]; then
    to_llvm_inp="/dev/stdin"
    to_llvm_out="${TEMP}"
    opt_inp="${TEMP}"
    opt_params="-S -o"
    opt_out="/dev/stdout"
elif [[ ${asm_out} = true ]]; then
   to_llvm_inp="/dev/stdin"
   to_llvm_out="${TEMP}"
   opt_inp="${TEMP}"
   opt_params="-o"
   opt_out="${TEMP}"
   llc_inp="${TEMP}"
   llc_out="${TEMP}.asm"
   llc_params="-mtriple=${mtriple} -o"
   clang_inp="${TEMP}.asm"
   clang_params="$LIB_OBJ -lm -o"
   clang_out="/dev/stdout"
else
  if [[ -z ${file_path} ]]; then
    echo "Usage: $0 [-O|-O0|-O1|O2|O3] -i|-f|<filename>"
    exit 1;
  fi
  echo "Compiling ${file_path}"
  file_name=${file_path%.*}
  to_llvm_inp="${file_path}"
  to_llvm_out="${TEMP}"
  opt_inp="${TEMP}"
  opt_params="-S -o"
  opt_out="${file_name}.imm"
  llc_inp="${file_name}.imm"
  llc_params="-mtriple=${mtriple} -o"
  llc_out="${TEMP}.asm"
  clang_inp="${TEMP}.asm"
  clang_params="$LIB_OBJ -lm -o"
  clang_out="${file_name}"
fi

if ! $pcl_compiler < ${to_llvm_inp} > ${to_llvm_out}; then
   echo "Error in compilation to llvm."
   exit 1
fi
if ! opt ${opt_flag} $opt_inp $opt_params $opt_out; then
   echo "Error in llvm optimization."
   exit 1
fi
if [[ ${ir_out} = true ]]; then
   rm $TEMP
   exit 0
fi
if ! llc $llc_inp $llc_params $llc_out; then
   echo "Error in compilation to assembly."
   exit 1
fi
if ! clang $clang_inp $clang_params $clang_out; then
   echo "Error in linking and compilation to executable."
   exit 1
fi
rm $TEMP
rm $TEMP.asm
