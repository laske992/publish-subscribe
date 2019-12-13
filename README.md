# publish-subscribe
Publish-Subscribe communication infrastructure

How to use this repo
1. clone to linux machine
2. navigate to project root directory
3. run 'make'
4. ./bin/server <port>  --> to run server
5. ./bin/client         --> to run single client instance (from other terminal)
  
  
######################################
makefile rules:
  make all          --> to compile all
  make remake       --> to clean and recompile all
  make clean        --> to clean binaries and objects
######################################
######################################
Server command:
  exit
#######################################
#######################################
Client commands:
  CONNECT:<port>:<name>
  SUBSCRIBE:<topic>
  PUBLISH:<topic>:<data to share>
  UNSUBSCRIBE:<topic>
  DISCONNECT

#######################################
