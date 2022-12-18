#!/bin/bash

function __wget() {
  IFS=/ read proto z host query <<< "$1"
  exec 3< /dev/tcp/$host/80
  {
    echo GET /$query HTTP/1.1
    echo connection: close
    echo host: $host
    echo
  } >&3 
  sed '1,/^$/d' <&3 > $(basename $1)
}

__wget http://neverssl.com
