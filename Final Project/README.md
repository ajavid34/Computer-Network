## For run project:
### 1. Compile server:
```
g++ −g −pthread message.h server.cpp main.cpp −o server
```
### 2. Compile client:
```
g++ −g −pthread message.h server.cpp client.cpp −o client
```
### 3. Run server:
```
. /server
```
### 4. Run client:
```
. /client localhost:9000 {client name}
```
