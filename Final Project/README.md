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
![image](https://user-images.githubusercontent.com/79438681/181439245-ad2bee7a-5fe7-4105-b7eb-7dc33939bec3.png)
