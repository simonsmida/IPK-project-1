# HTTP server in C

🏷️: IPK-project1
🧑🏻‍💻: *Šimon Šmída* (xsmida03)\
✉️: *xsmida03@vutbr.cz* 

Implementation of a server communicating via [HTTP](https://tools.ietf.org/html/rfc7231) in C programming language. The server provides information about the system. The server will be listening on the given port and according to the url it will return demanded information. Communication with this server is possible via a web browser, and by using `wget` and `curl` tools as well. 


The server is able to process 3 types of requests, which are sent using the `GET` method:
  - getting **domain name**
  - getting **CPU information**
  - getting **CPU load** [%]

---

## Usage
#### Start the server
Server is waiting (actively) for incoming connections (requests) on the specified port
```
$ ./hinfosvc <port>
```


### Example 1
The server is expected to be already running

1. Getting **domain name**
      - server returns a computer network name, including its domain, e.g.:
      ```
      $ GET http://servername:12345/hostname
      merlin.fit.vutbr.cz
      ```
  2. Getting **CPU information**
      - server returns information about a CPU, e.g.:
      ```
      $ GET http://servername:12345/cpu-name
      Intel(R) Xeon(R) CPU E5-2640 0 @ 2.50GHz
      ```
  3. Getting **CPU load** [%]
      - server returns information about a current CPU load, e.g.:
      ```
      $ GET http://servername:12345/load
      65%
      ```

### Example 2
Using `curl`
```
$ ./hinfosvc 12345 &
    curl http://localhost:12345/hostname
    curl http://localhost:12345/cpu-name
    curl http://localhost:12345/load
```
